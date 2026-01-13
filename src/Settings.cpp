namespace Settings
{
    REX::INI::SettingStore *settingStore;

    void SetLogLevel()
    {
        switch (log_level.GetValue())
        {
        case 0:
            spdlog::set_level(spdlog::level::err);
            spdlog::flush_on(spdlog::level::err);
            break;
        case 1:
            spdlog::set_level(spdlog::level::warn);
            spdlog::flush_on(spdlog::level::warn);
            break;
        case 2:
            spdlog::set_level(spdlog::level::info);
            spdlog::flush_on(spdlog::level::info);
            break;
        case 3:
            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::debug);
            break;
        case 4:
            spdlog::set_level(spdlog::level::trace);
            spdlog::flush_on(spdlog::level::trace);
            break;
        default:
            log_level.SetValue(2);
            spdlog::set_level(spdlog::level::info);
            spdlog::flush_on(spdlog::level::info);
        }
    }

    void InitINI()
    {
        settingStore = REX::INI::SettingStore::GetSingleton();
        settingStore->Init("Data/SKSE/Plugins/MCMShortcutNG.ini", "Data/SKSE/Plugins/MCMShortcutNG.ini");
    }

    void LoadJSON()
    {
        std::ifstream inputFile("Data\\SKSE\\Plugins\\MCMShortcutNG.json");
        if (!inputFile.is_open())
        {
            logger::error("Error: Could not open the file MCMShortcutNG.json");
            return;
        }
        json jsonData;
        try
        {
            jsonData = json::parse(inputFile);
        }
        catch (json::parse_error &e)
        {
            logger::error("Error parsing json: {}", e.what());
            return;
        }

        inputFile.close();

        shortcutInfos.clear();
        for (auto hotkeyConfig : jsonData)
        {
            shorcutInfo newInfo;

            if (hotkeyConfig.contains("Modifier 1"))
                newInfo.modifier1 = hotkeyConfig["Modifier 1"].get<int>();
            if (hotkeyConfig.contains("Modifier 2"))
                newInfo.modifier2 = hotkeyConfig["Modifier 2"].get<int>();
            if (hotkeyConfig.contains("Hotkey"))
                newInfo.hotkey = hotkeyConfig["Hotkey"].get<int>();

            if (hotkeyConfig.contains("Mod Name"))
            {
                newInfo.modName = hotkeyConfig["Mod Name"].get<std::string>();
                newInfo.openMod = std::strcmp(newInfo.modName.c_str(), "None") != 0;
                if (newInfo.openMod)
                {
                    if (newInfo.modName.starts_with("$"))
                        SKSE::Translation::Translate(newInfo.modName, newInfo.modNameTranslated);
                    else
                        newInfo.modNameTranslated = newInfo.modName;
                }
            }
            if (hotkeyConfig.contains("Mod Open Delay"))
                newInfo.modDelay = hotkeyConfig["Mod Open Delay"].get<int>();

            if (hotkeyConfig.contains("Page Name"))
            {
                newInfo.pageName = hotkeyConfig["Page Name"].get<std::string>();
                newInfo.openPage = std::strcmp(newInfo.pageName.c_str(), "None") != 0;
                /*if (newInfo.openPage)
                {
                    //if (newInfo.pageName.starts_with("$"))
                        SKSE::Translation::Translate(newInfo.pageName, newInfo.pageNameTranslated);
                    else
                        newInfo.pageNameTranslated = newInfo.pageName;
                }*/
            }
            if (hotkeyConfig.contains("Page Open Delay"))
                newInfo.pageDelay = hotkeyConfig["Page Open Delay"].get<int>();

            if (newInfo.hotkey != 0)
                shortcutInfos.push_back(newInfo);
        }
    }

    bool ValidateShortcuts()
    {
        std::vector<std::array<int, 3>> keyCombos;
        bool invalid = false;
        std::string message;
        for (shorcutInfo info : shortcutInfos)
        {
            logger::info("Shortcut Info:");
            logger::info(">  Keys:  Mod1: {},   Mod2: {},   Key: {},", info.modifier1, info.modifier2, info.hotkey);
            logger::info(">  Mod:   Open: {},   Name: {},   Delay: {}", info.openMod, info.modName, info.modDelay);
            logger::info(">  Page:  Open: {},   Name: {},   Delay: {}", info.openPage, info.pageName, info.pageDelay);

            std::array<int, 3> combo = {info.modifier1, info.modifier2, info.hotkey};
            std::sort(combo.begin(), combo.end());
            auto iter = std::find(keyCombos.begin(), keyCombos.end(), combo);

            if (info.hotkey == 0)
            {
                invalid = true;
                message =
                    "You have an invalid shortcut defined in your MCMShortcut.json. No hotkey value may be 0, only modifiers. "
                    "A default shortcut has been set in game as Shift+Ctrl+V. Please fix the invalid shortcut in the json and "
                    "then reload the shortcuts by holding Shift+Ctrl+V past your setting threshold defined in the MCMShortcutNG.ini "
                    "(default 3 seconds).";
                break;
            }

            if (iter == keyCombos.end())
                keyCombos.push_back(combo);
            else
            {
                invalid = true;
                message =
                    "You have at least two shortcuts with the same keys (order doesn't matter) which can cause "
                    "unintended behavior. All shortcuts have been cleared and one has been created as "
                    "Shift+Ctrl+V in game. Please fix the shortcut confict in your MCMShortcut.json and then "
                    "reload the shortcuts by holding Shift+Ctrl+V past your setting threshold defined "
                    "in the MCMShortcutNG.ini (default 3 seconds).";
                break;
            }
        }
        if (!invalid && shortcutInfos.size() == 0)
        {
            invalid = true;
            message =
                "You have no shortcuts defined in your MCMShortcut.json. A default shortcut has been set in game "
                "as Shift+Ctrl+V. Please define your own shortcut in the json and then reload the shortcuts by "
                "holding Shift+Ctrl+V past your setting threshold defined in the MCMShortcutNG.ini (default 3 seconds).";
        }
        if (invalid)
        {
            shortcutInfos.clear();
            shorcutInfo info{42, 29, 47};
            shortcutInfos.push_back(info);
            RE::DebugMessageBox(message.c_str());
            return false;
        }
        return true;
    }

    void LoadINI()
    {
        settingStore->Load();
        SetLogLevel();
        settingStore->Save();
    }
}