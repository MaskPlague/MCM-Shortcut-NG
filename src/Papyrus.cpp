namespace Papyrus
{
    bool OpenMCM(RE::StaticFunctionTag *, std::string modName, int modDelay, std::string pageName, int pageDelay)
    {
        if (MCMManager::lock)
        {
            logger::trace("MCM management currently locked due to already opening an MCM, mod, or page."sv);
            return false;
        }
        logger::debug("Opening an MCM from papyrus call, Mod: {}, Mod Delay: {}, Page: {}, Page Delay: {}"sv,
                      modName.c_str(), modDelay, pageName.c_str(), pageDelay);
        papyrusInfo->modName = modName;
        papyrusInfo->openMod = std::strcmp(papyrusInfo->modName.c_str(), "None") != 0;
        if (papyrusInfo->openMod)
        {
            if (papyrusInfo->modName.starts_with("$"))
                SKSE::Translation::Translate(modName, papyrusInfo->modNameTranslated);
            else
                papyrusInfo->modNameTranslated = papyrusInfo->modName;
        }
        papyrusInfo->modDelay = modDelay;
        papyrusInfo->pageName = pageName;
        papyrusInfo->openPage = std::strcmp(papyrusInfo->pageName.c_str(), "None") != 0;
        papyrusInfo->pageDelay = pageDelay;
        MCMManager::currentInfo = papyrusInfo;
        MCMManager::CloseOpenMenus();
        return true;
    }

    bool FixKeys(RE::StaticFunctionTag *)
    {
        MCMManager::FixKeyRepeat();
        return true;
    }

    bool IsInstalled(RE::StaticFunctionTag *)
    {
        return true;
    }

    void Register(RE::BSScript::IVirtualMachine *vm)
    {
        vm->RegisterFunction(
            "OpenMCM",
            "MCMShortcutNG",
            OpenMCM);
        vm->RegisterFunction(
            "FixKeys",
            "MCMShortcutNG",
            FixKeys);
        vm->RegisterFunction(
            "IsInstalled",
            "MCMShortcutNG",
            IsInstalled);
        logger::info("Registered MCMShortcutNG's OpenMenu papyrus function");
    }
}