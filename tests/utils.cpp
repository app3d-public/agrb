#include <agrb/buffer.hpp>
#include <agrb/utils.hpp>
#include "env.hpp"

using namespace agrb;

void test_utils()
{
    Enviroment env;
    init_environment(env);

    const u32 width = 16, height = 16, pixel_size = 4;
    const vk::DeviceSize image_size = width * height * pixel_size;

    buffer src, dst;
    for (buffer *b : {&src, &dst})
    {
        b->instance_count = 1;
        b->vk_usage_flags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
        b->vma_usage_flags = VMA_MEMORY_USAGE_GPU_ONLY;
        b->memory_property_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        b->priority = 0.5f;

        construct_buffer(*b, image_size);
        assert(allocate_buffer(*b, env.d));
    }

    // copy_buffer
    copy_buffer(env.d, src.vk_buffer, dst.vk_buffer, image_size);

    // get_alignment
    size_t aligned = get_alignment(20, 16);
    assert(aligned % 16 == 0 && aligned >= 20);

    // Image creation
    vk::Image image;
    VmaAllocation alloc;

    vk::ImageCreateInfo img_info{};
    img_info.imageType = vk::ImageType::e2D;
    img_info.extent = vk::Extent3D{width, height, 1};
    img_info.mipLevels = 1;
    img_info.arrayLayers = 1;
    img_info.format = vk::Format::eR8G8B8A8Unorm;
    img_info.tiling = vk::ImageTiling::eOptimal;
    img_info.initialLayout = vk::ImageLayout::eUndefined;
    img_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    img_info.sharingMode = vk::SharingMode::eExclusive;
    img_info.samples = vk::SampleCountFlagBits::e1;

    assert(create_image(img_info, image, alloc, env.d.allocator));

    // Transition: Undefined → TransferDstOptimal
    assert(transition_image_layout(env.d, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                   1) == vk::Result::eSuccess);

    // Copy buffer → image
    assert(copy_buffer_to_image(env.d, src.vk_buffer, image, 1, {width, height, 1}) == vk::Result::eSuccess);

    // Transition: TransferDstOptimal → TransferSrcOptimal
    assert(transition_image_layout(env.d, image, vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eTransferSrcOptimal, 1) == vk::Result::eSuccess);

    // Copy image → buffer
    copy_image_to_buffer(env.d, dst.vk_buffer, image, {width, height}, 1);

    // Clear
    destroy_buffer(src, env.d);
    destroy_buffer(dst, env.d);
    env.d.vk_device.destroyImage(image, nullptr, env.d.loader);
    vmaFreeMemory(env.d.allocator, alloc);
}