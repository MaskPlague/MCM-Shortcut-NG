// To keep intellisense happy
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include "REX/REX/INI.h"
#include "REX/REX/Setting.h"
#include "json.hpp"
#include "Settings.h"
#include "MCMManager.h"
#include "Events.h"
#include "Papyrus.h"
// ---------------------------

namespace MCMShortcutNG
{
    void MessageListener(SKSE::MessagingInterface::Message *msg)
    {
        if (msg->type == SKSE::MessagingInterface::kDataLoaded)
        {
            logger::info("Adding Event Sinks"sv);
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(Events::InputEvent::GetSingleton());
            RE::UI::GetSingleton()->AddEventSink(Events::UIEvent::GetSingleton());

            Settings::LoadJSON();
        }
        else if (msg->type == SKSE::MessagingInterface::kPostLoadGame && bool(msg->data))
        {
            Settings::gameHasLoaded = true;
            Settings::ValidateShortcuts();
        }
    }

    bool RegisterPapyrus(RE::BSScript::IVirtualMachine *vm)
    {
        logger::info("Registering for papyrus...");
        Papyrus::Register(vm);
        return true;
    }

    extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface *a_skse)
    {
        SKSE::Init(a_skse);
        logger::info("MCM Shortcut NG Starting"sv);
        Settings::InitINI();
        Settings::LoadINI();
        SKSE::GetMessagingInterface()->RegisterListener(MessageListener);
        SKSE::GetPapyrusInterface()->Register(RegisterPapyrus);
        logger::info("MCM Shortcut NG Plugin Loaded"sv);
        return true;
    }
}