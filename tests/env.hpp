#pragma once

#include <agrb/device.hpp>

struct Enviroment
{
    acul::task::service_dispatch sd;
    agrb::device d;
    agrb::device_runtime_data rd;
};

void init_environment(Enviroment& env);