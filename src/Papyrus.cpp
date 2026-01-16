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
        MCMManager::currentInfo = {};
        MCMManager::currentInfo.modName = modName;
        MCMManager::currentInfo.openMod = std::strcmp(MCMManager::currentInfo.modName.c_str(), "None") != 0;
        if (MCMManager::currentInfo.openMod)
        {
            if (MCMManager::currentInfo.modName.starts_with("$"))
                SKSE::Translation::Translate(modName, MCMManager::currentInfo.modNameTranslated);
            else
                MCMManager::currentInfo.modNameTranslated = MCMManager::currentInfo.modName;
        }
        MCMManager::currentInfo.modDelay = modDelay;
        MCMManager::currentInfo.pageName = pageName;
        MCMManager::currentInfo.openPage = std::strcmp(MCMManager::currentInfo.pageName.c_str(), "None") != 0;
        MCMManager::currentInfo.pageDelay = pageDelay;
        MCMManager::CloseOpenMenus();
        return true;
    }

    bool FixKeys(RE::StaticFunctionTag *)
    {
        MCMManager::FixKeyRepeat();
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
        logger::info("Registered MCMShortcutNG's OpenMenu papyrus function");
    }
}