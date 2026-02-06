#include <agrb/utils/buffer.hpp>
#include "env.hpp"

using namespace agrb;

void check_buffer_construct(device &d)
{
    buffer b;
    b.instance_count = 1;
    auto create_info = make_alloc_info(VMA_MEMORY_USAGE_CPU_ONLY, vk::MemoryPropertyFlagBits::eHostVisible,
                                       vk::MemoryPropertyFlagBits::eHostCoherent, 0.1f);
    construct_buffer(b, sizeof(int));
    assert(allocate_buffer(b, create_info, vk::BufferUsageFlagBits::eTransferSrc, d));
    assert(map_buffer(b, d));

    int value = 7;
    write_to_buffer(b, &value);
    flush_buffer(b, d);
    assert(invalidate_buffer(b, d) == vk::Result::eSuccess);
    int *mapped = (int *)b.mapped;
    assert(mapped);
    assert(*mapped == value);

    unmap_buffer(b, d);
    destroy_buffer(b, d);
}

void check_buffer_ubo(device &d)
{
    buffer b;
    b.instance_count = 3;
    auto create_info = make_alloc_info(VMA_MEMORY_USAGE_CPU_ONLY, vk::MemoryPropertyFlagBits::eHostVisible,
                                       vk::MemoryPropertyFlagBits::eHostCoherent, 0.1f);

    construct_ubo_buffer(b, sizeof(u32), d.rd);
    assert(allocate_buffer(b, create_info, vk::BufferUsageFlagBits::eTransferSrc, d));
    assert(map_buffer(b, d));

    u32 values[3] = {1, 2, 3};
    for (int i = 0; i < 3; ++i)
    {
        write_to_buffer_index(b, sizeof(u32), &values[i], i);
        flush_buffer_index(b, i, d);
        assert(invalidate_buffer_index(b, i, d) == vk::Result::eSuccess);
    }

    unmap_buffer(b, d);
    destroy_buffer(b, d);
}

void check_move_to_buffer(device &d)
{
    buffer b;
    b.instance_count = 1;
    auto create_info = make_alloc_info(VMA_MEMORY_USAGE_CPU_ONLY, vk::MemoryPropertyFlagBits::eHostVisible,
                                       vk::MemoryPropertyFlagBits::eHostCoherent, 0.1f);

    construct_buffer(b, sizeof(int) * 3);
    assert(allocate_buffer(b, create_info, vk::BufferUsageFlagBits::eTransferSrc, d));
    assert(map_buffer(b, d));

    int values[] = {10, 20, 30};
    move_to_buffer(b, values, sizeof(values));
    flush_buffer(b, d);

    int *mapped = static_cast<int *>(b.mapped);
    assert(mapped[0] == 10);
    assert(mapped[1] == 20);
    assert(mapped[2] == 30);

    int subvalue = 99;
    move_to_buffer(b, &subvalue, sizeof(subvalue), sizeof(int));
    flush_buffer_index(b, 1, d);
    assert(mapped[1] == 99);

    unmap_buffer(b, d);

    // Defered release
    buffer_mem_cache cache(b, d);
    cache.on_free();
    assert(b.vk_buffer == VK_NULL_HANDLE);
}

void test_buffer()
{
    init_library();
    Enviroment env;
    init_environment(env);
    check_buffer_construct(env.d);
    check_buffer_ubo(env.d);
    check_move_to_buffer(env.d);
    destroy_device(env.d);
    destroy_library();
}