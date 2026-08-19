#include <agrb/dynamic_buffer.hpp>
#include <agrb/utils/buffer.hpp>
#include "env.hpp"

using namespace agrb;

void test_dynamic_buffer()
{
    init_library();
    Enviroment env;
    init_environment(env);

    managed_buffer configuration;
    configuration.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc |
                                 vk::BufferUsageFlagBits::eTransferDst;
    configuration.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    configuration.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    dynamic_buffer data(env.d, configuration);
    const int values[] = {10, 20, 30};
    assert(data.resize(sizeof(values)));
    assert(data.size() == sizeof(values));
    assert(data.capacity() >= data.size());

    assert(map_buffer(data.data(), env.d));
    write_to_buffer(data.data(), const_cast<int *>(values), sizeof(values));
    unmap_buffer(data.data(), env.d);

    const auto old_capacity = data.capacity();
    assert(data.resize(old_capacity + 1u));
    assert(data.capacity() > old_capacity);
    assert(map_buffer(data.data(), env.d));
    assert(invalidate_buffer(data.data(), env.d) == vk::Result::eSuccess);
    const auto *preserved = static_cast<const int *>(data.data().mapped);
    assert(preserved[0] == values[0]);
    assert(preserved[1] == values[1]);
    assert(preserved[2] == values[2]);
    unmap_buffer(data.data(), env.d);

    data.destroy();
    destroy_device(env.d);
    destroy_library();
}
