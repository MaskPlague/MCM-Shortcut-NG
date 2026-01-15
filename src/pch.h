#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
namespace logger = SKSE::log;
#define DLLEXPORT __declspec(dllexport)
#include "REX/REX/INI.h"
#include "REX/REX/Setting.h"
#include "json.hpp"
using json = nlohmann::json;
#include "Settings.h"
#include "MCM.h"
#include "Events.h"
using namespace std::literals;

/*struct ScopedTimer
{
    std::string_view name;
    std::chrono::high_resolution_clock::time_point start;

    ScopedTimer(std::string_view timerName) : name(timerName)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    ~ScopedTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        // Calculate in microseconds (1/1,000,000th of a second)
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        logger::debug("[BENCHMARK] {} took: {} microseconds", name, duration);
    }
};*/