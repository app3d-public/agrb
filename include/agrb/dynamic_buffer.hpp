#pragma once

#include "buffer.hpp"

namespace agrb
{
    /**
     * A growable Vulkan buffer without host-access guarantees.
     *
     * Size and capacity are expressed in bytes. Growing the allocation preserves
     * the current contents with a GPU buffer copy, so the configured usage must
     * contain eTransferSrc and eTransferDst once reallocation is possible.
     */
    class AGRB_EXPORT dynamic_buffer
    {
    public:
        using size_type = vk::DeviceSize;

        dynamic_buffer() = default;
        dynamic_buffer(device &device, const managed_buffer &configuration);
        dynamic_buffer(const dynamic_buffer &) = delete;
        dynamic_buffer &operator=(const dynamic_buffer &) = delete;
        dynamic_buffer(dynamic_buffer &&other) noexcept;
        dynamic_buffer &operator=(dynamic_buffer &&other) = delete;
        ~dynamic_buffer();

        void init(device &device, const managed_buffer &configuration);
        void destroy();

        bool reserve(size_type new_capacity);
        bool resize(size_type new_size);
        void clear() { _size = 0; }

        bool empty() const { return _size == 0; }
        bool is_inited() const { return _device != nullptr && _data.vk_buffer; }
        size_type size() const { return _size; }
        size_type capacity() const { return _data.buffer_size; }

        buffer &data() { return _data; }
        const buffer &data() const { return _data; }

    private:
        device *_device = nullptr;
        managed_buffer _data;
        size_type _size = 0;

        bool allocate();
        bool reallocate(size_type new_capacity);
    };
} // namespace agrb
