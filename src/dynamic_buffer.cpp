#include <acul/memory/alloc.hpp>
#include <agrb/dynamic_buffer.hpp>
#include <agrb/utils/buffer.hpp>

namespace agrb
{
    dynamic_buffer::dynamic_buffer(device &device, const managed_buffer &configuration) { init(device, configuration); }

    dynamic_buffer::dynamic_buffer(dynamic_buffer &&other) noexcept
        : _device(other._device), _data(other._data), _size(other._size)
    {
        other._device = nullptr;
        other._data = {};
        other._size = 0;
    }

    dynamic_buffer::~dynamic_buffer() { destroy(); }

    void dynamic_buffer::init(device &device, const managed_buffer &configuration)
    {
        assert(!_device && !_data.vk_buffer);
        _device = &device;
        _data = configuration;
        _size = 0;
        construct_buffer(_data, 1u);
        if (_data.instance_count > 0 && !allocate()) throw acul::bad_alloc(_data.buffer_size);
    }

    void dynamic_buffer::destroy()
    {
        if (_data.vk_buffer) destroy_buffer(_data, *_device);
        _device = nullptr;
        _size = 0;
    }

    bool dynamic_buffer::reserve(size_type new_capacity)
    {
        if (new_capacity <= capacity()) return true;
        return reallocate(new_capacity);
    }

    bool dynamic_buffer::resize(size_type new_size)
    {
        if (new_size > capacity())
        {
            const size_type new_capacity = acul::get_growth_size(capacity(), new_size);
            if (!reallocate(new_capacity)) return false;
        }
        _size = new_size;
        return true;
    }

    bool dynamic_buffer::allocate()
    {
        assert(_device && _data.buffer_size > 0);
        const auto alloc_info =
            make_alloc_info(_data.vma_usage, _data.required_flags, _data.prefered_flags, _data.priority);
        return allocate_buffer(_data, alloc_info, _data.buffer_usage, *_device);
    }

    bool dynamic_buffer::reallocate(size_type new_capacity)
    {
        assert(_device && new_capacity > capacity());
        managed_buffer new_buffer = _data;
        new_buffer.mapped = nullptr;
        new_buffer.vk_buffer = VK_NULL_HANDLE;
        new_buffer.allocation = VK_NULL_HANDLE;
        new_buffer.instance_count = 1u;
        construct_buffer(new_buffer, new_capacity);

        const auto alloc_info = make_alloc_info(new_buffer.vma_usage, new_buffer.required_flags,
                                                new_buffer.prefered_flags, new_buffer.priority);
        if (!allocate_buffer(new_buffer, alloc_info, new_buffer.buffer_usage, *_device)) return false;

        if (_data.vk_buffer && _size > 0)
        {
            const bool can_copy = static_cast<bool>(_data.buffer_usage & vk::BufferUsageFlagBits::eTransferSrc) &&
                                  static_cast<bool>(new_buffer.buffer_usage & vk::BufferUsageFlagBits::eTransferDst);
            assert(can_copy && "agrb::dynamic_buffer growth requires transfer source/destination usage");
            if (!can_copy)
            {
                destroy_buffer(new_buffer, *_device);
                return false;
            }

            single_time_exec exec{*_device};
            copy_buffer(exec, *_device, _data.vk_buffer, new_buffer.vk_buffer, _size);
            if (exec.end() != vk::Result::eSuccess)
            {
                destroy_buffer(new_buffer, *_device);
                return false;
            }
        }

        if (_data.vk_buffer) destroy_buffer(_data, *_device);
        _data = new_buffer;
        return true;
    }
} // namespace agrb
