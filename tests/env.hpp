#pragma once

#include <agrb/device.hpp>

struct Enviroment
{
    agrb::device d;
    agrb::device_runtime_data rd;
};

void init_environment(Enviroment& env);