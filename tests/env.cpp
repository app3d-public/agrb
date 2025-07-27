#include "env.hpp"

using namespace agrb;

void init_environment(Enviroment &env)
{
    env.sd.run();
    auto *service = acul::alloc<acul::log::log_service>();
    env.sd.register_service(service);
    service->level = acul::log::level::trace;

    auto *console = service->add_logger<acul::log::console_logger>("console");
    service->default_logger = console;
    console->set_pattern("%(message)\n");
    assert(console->name() == "console");

    device_create_ctx ctx;
    ctx.set_validation_layers({"VK_LAYER_KHRONOS_validation"})
        .set_device_extensions_optional(
            {vk::EXTMemoryPriorityExtensionName, vk::EXTPageableDeviceLocalMemoryExtensionName})
        .set_fence_pool_size(8)
        .set_runtime_data(&env.rd);
    init_device("app_test", 1, env.d, &ctx);
    assert(env.d.vk_device);
    assert(env.d.physical_device);
    assert(env.rd.get_device_properties().vendorID != 0);
}