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
