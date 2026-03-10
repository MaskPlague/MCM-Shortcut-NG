#pragma once

namespace Papyrus
{
    inline bool awaitingMenu = false;

    bool OpenMCM(RE::StaticFunctionTag *, std::string modName, int modDelay, std::string pageName, int pageDelay);

    void Register(RE::BSScript::IVirtualMachine *vm);

    inline Settings::shortcutInfo *papyrusInfo = new Settings::shortcutInfo(0, 0, 0);
}