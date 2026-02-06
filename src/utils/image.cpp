#include <agrb/utils/image.hpp>

namespace agrb
{
    void transition_image_layout(single_time_exec &exec, vk::Image image, vk::ImageLayout old_layout,
                                 vk::ImageLayout new_layout, u32 mipLevels)
    {

        vk::ImageMemoryBarrier barrier{};
        barrier.setOldLayout(old_layout)
            .setNewLayout(new_layout)
            .setImage(image)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
        vk::PipelineStageFlagBits src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        vk::PipelineStageFlagBits dst_stage = vk::PipelineStageFlagBits::eTopOfPipe;

        switch (old_layout)
        {
            case vk::ImageLayout::eUndefined:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
                break;
            case vk::ImageLayout::eTransferDstOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
                src_stage = vk::PipelineStageFlagBits::eTransfer;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
                src_stage = vk::PipelineStageFlagBits::eTransfer;
                break;
            case vk::ImageLayout::eReadOnlyOptimal:
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
                src_stage = vk::PipelineStageFlagBits::eFragmentShader;
                break;
            default:
                throw acul::runtime_error("Unsupported src layout transition");
        }

        switch (new_layout)
        {
            case vk::ImageLayout::eTransferDstOptimal:
                barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                dst_stage = vk::PipelineStageFlagBits::eTransfer;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
                src_stage = vk::PipelineStageFlagBits::eTransfer;
                dst_stage = vk::PipelineStageFlagBits::eHost;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
                dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                break;
            case vk::ImageLayout::eReadOnlyOptimal:
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
                barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
                break;
            case vk::ImageLayout::eGeneral:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
                barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
                src_stage = vk::PipelineStageFlagBits::eTransfer;
                dst_stage = vk::PipelineStageFlagBits::eTransfer;
                break;
            default:
                throw acul::runtime_error("Unsupported dst layout transition");
        }

        exec.command_buffer.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1,
                                            &barrier, exec.loader);
    }
} // namespace agrb