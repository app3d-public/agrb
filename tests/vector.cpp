#include <agrb/vector.hpp>
#include <numeric>
#include "env.hpp"

using namespace agrb;

void test_vector_basic(device &d)
{
    vector<int> v;
    assert(v.empty());
    assert(v.size() == 0);

    managed_buffer b0;
    b0.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b0.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b0.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    v.init(d, b0);

    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    assert(!v.empty());
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);

    v.clear();
    assert(v.empty());
    assert(v.size() == 0);

    managed_buffer b1;
    b1.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b1.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b1.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    b1.instance_count = 10;
    vector<u32> v2(d, b1, 7);
    assert(v2.size() == 10);
    assert(v2[0] == 7);
}

void test_vector_resize_reserve(device &d)
{
    vector<int> v;
    managed_buffer b0;
    b0.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b0.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b0.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    v.init(d, b0);

    v.reserve(10);
    assert(v.capacity() >= 10);
    assert(v.size() == 0);

    v.resize(5);
    assert(v.size() == 5);

    v[2] = 42;
    assert(v[2] == 42);

    v.resize(2);
    assert(v.size() == 2);
}

void test_vector_move(device &d)
{
    managed_buffer b;
    b.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vector<int> v1{d, b};
    v1.push_back(100);
    v1.push_back(200);

    // Move
    vector<int> v4 = std::move(v1);
    assert(v4.size() == 2);
    assert(v4[0] == 100);
    assert(v4[1] == 200);
    assert(v1.empty() || v1.size() == 0);
}

void test_vector_front_back(device &d)
{
    managed_buffer b;
    b.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vector<int> v{d, b};
    v.push_back(5);
    v.push_back(10);
    v.push_back(15);

    assert(v.front() == 5);
    assert(v.back() == 15);

    v.front() = 1;
    v.back() = 99;

    assert(v.front() == 1);
    assert(v.back() == 99);
}

void test_vector_insert_erase(device &d)
{
    managed_buffer b;
    b.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vector<int> v{{1, 2, 4}, d, b};

    v.insert(v.begin() + 2, 3);
    assert(v.size() == 4);
    assert(v[2] == 3);
    assert(v[3] == 4);

    v.erase(v.begin() + 1);
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 3);
    assert(v[2] == 4);
}

void test_vector_assign(device &d)
{
    managed_buffer b;
    b.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vector<int> v{d, b};
    v.assign(5, 42);
    assert(v.size() == 5);
    for (auto &e : v) assert(e == 42);
}

void test_vector_iterators(device &d)
{
    managed_buffer b;
    b.buffer_usage = vk::BufferUsageFlagBits::eStorageBuffer;
    b.vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    b.required_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    b.instance_count = 5;
    vector<int> v(d, b);
    assert(v.data().mapped);
    std::iota(v.begin(), v.end(), 10);

    assert(std::find(v.begin(), v.end(), 12) != v.end());
    assert(std::find(v.begin(), v.end(), 999) == v.end());

    auto it = std::find_if(v.begin(), v.end(), [](int x) { return x > 12; });
    assert(it != v.end());
    assert(*it == 13);
}

void test_vector()
{
    init_library();
    Enviroment env;
    init_environment(env);

    test_vector_basic(env.d);
    test_vector_resize_reserve(env.d);
    test_vector_move(env.d);
    test_vector_front_back(env.d);
    test_vector_insert_erase(env.d);
    test_vector_assign(env.d);
    test_vector_iterators(env.d);
    destroy_library();
}