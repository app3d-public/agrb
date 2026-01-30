#include "env.hpp"

using namespace agrb;

void init_environment(Enviroment &env)
{
    device_create_ctx ctx;
    ctx.set_device_extensions_optional(
           {vk::EXTMemoryPriorityExtensionName, vk::EXTPageableDeviceLocalMemoryExtensionName})
        .set_fence_pool_size(8)
        .set_runtime_data(&env.rd);
    init_device("app_test", 1, env.d, &ctx);
    assert(env.d.vk_device);
    assert(env.d.physical_device);
    assert(env.rd.get_device_properties().vendorID != 0);
}