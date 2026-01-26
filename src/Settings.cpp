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
        logger::info("Loading shortcuts from MCMShortcutNG.json");
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
            shortcutInfo newInfo;
            if (hotkeyConfig.contains("_KeyCodeComment"))
                continue;

            if (hotkeyConfig.contains("Modifier 1"))
                newInfo.modifier1 = hotkeyConfig["Modifier 1"].get<int>();
            if (hotkeyConfig.contains("Modifier 2"))
                newInfo.modifier2 = hotkeyConfig["Modifier 2"].get<int>();
            if (hotkeyConfig.contains("Hotkey"))
                newInfo.hotkey = hotkeyConfig["Hotkey"].get<int>();

            if (hotkeyConfig.contains("Key Order Matters"))
                newInfo.orderMatters = hotkeyConfig["Key Order Matters"].get<bool>();

            if (hotkeyConfig.contains("Disable In Inventory"))
                newInfo.disableInInventory = hotkeyConfig["Disable In Inventory"].get<bool>();

            if (hotkeyConfig.contains("Disable In Dialogue"))
                newInfo.disableInDialogue = hotkeyConfig["Disable In Dialogue"].get<bool>();

            if (hotkeyConfig.contains("Disable In Magic"))
                newInfo.disableInMagic = hotkeyConfig["Disable In Magic"].get<bool>();

            if (hotkeyConfig.contains("Disable In Map"))
                newInfo.disableInMap = hotkeyConfig["Disable In Map"].get<bool>();

            if (hotkeyConfig.contains("Disable In Favorites"))
                newInfo.disableInFavorites = hotkeyConfig["Disable In Favorites"].get<bool>();

            if (hotkeyConfig.contains("Can Reload Settings"))
                newInfo.canReloadSettings = hotkeyConfig["Can Reload Settings"].get<bool>();

            if (hotkeyConfig.contains("Controller"))
                newInfo.controller = hotkeyConfig["Controller"].get<bool>();

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
            }
            if (hotkeyConfig.contains("Page Open Delay"))
                newInfo.pageDelay = hotkeyConfig["Page Open Delay"].get<int>();

            if (newInfo.hotkey != 0)
                shortcutInfos.push_back(newInfo);
        }
        logger::info("Successfully loaded shortcuts from JSON");
    }

    struct ShortcutKeySet
    {
        bool controller;
        std::set<int> keys;

        bool operator==(const ShortcutKeySet &other) const
        {
            return controller == other.controller && keys == other.keys;
        }

        bool operator<(const ShortcutKeySet &other) const
        {
            if (controller != other.controller)
                return controller < other.controller;
            return keys < other.keys;
        }
    };

    ShortcutKeySet MakeKeySet(const shortcutInfo &info)
    {
        ShortcutKeySet out;
        out.controller = info.controller;

        if (info.modifier1 != 0)
            out.keys.insert(info.modifier1);

        if (info.modifier2 != 0)
            out.keys.insert(info.modifier2);

        out.keys.insert(info.hotkey);

        return out;
    }

    bool ValidateShortcuts()
    {
        logger::info("Validating shortcuts\n");
        std::set<ShortcutKeySet> seen;
        bool invalid = false;
        std::string message;
        for (shortcutInfo info : shortcutInfos)
        {
            logger::info("Shortcut Info:        Can Reload Settings: {},    Controller: {}", info.canReloadSettings, info.controller);
            logger::info(">  Keys:  Mod1: {},   Mod2: {},   Key: {},    Order Matters: {},", info.modifier1, info.modifier2, info.hotkey, info.orderMatters);
            logger::info(">  Mod:   Open: {},   Name: {},   Delay: {}", info.openMod, info.modName, info.modDelay);
            logger::info(">  Page:  Open: {},   Name: {},   Delay: {}", info.openPage, info.pageName, info.pageDelay);
            logger::info(">  Disabled:   Inv: {},    Dia: {},    Mag: {},    Map: {}",
                         info.disableInInventory, info.disableInDialogue, info.disableInMagic, info.disableInMap);
            logger::info("> -- --  -  -- --  -  -- --  -  -- --  -  -- --  -  -- --  -  -- --  -  -- -- <\n");

            if (!info.hotkey)
            {
                invalid = true;
                message = "- MCM Shortcut NG -\nHotkey may not be 0, only modifier keys can.";
                break;
            }

            auto keys = MakeKeySet(info);

            if (!seen.insert(keys).second)
            {
                invalid = true;
                message =
                    "- MCM Shortcut NG -\nTwo shortcuts on the same input device resolve to the same "
                    "key combination. This would cause ambiguous activation.";
                break;
            }
        }
        if (invalid)
        {
            shortcutInfos.clear();
            shortcutInfo info{42, 29, 47};
            shortcutInfos.push_back(info);
            message = message + " A default shortcut has been set in game as Shift+Ctrl+V. Please fix this issue and "
                                "then reload the shortcuts by holding Shift+Ctrl+V past your setting threshold defined in the MCMShortcutNG.ini "
                                "(default 3 seconds).";
            RE::DebugMessageBox(message.c_str());
            logger::warn("Shortcuts failed validation!");
            return false;
        }
        logger::info("Shortcuts are valid");
        return true;
    }

    void GetKeyRepeatLongAndShort()
    {
        if (auto fMenuKeyRepeatLong = RE::GetINISetting("fMenuKeyRepeatLong:Interface"))
            keyRepeatLong = fMenuKeyRepeatLong->GetFloat();
        if (auto fMenuKeyRepeatShort = RE::GetINISetting("fMenuKeyRepeatShort:Interface"))
            keyRepeatShort = fMenuKeyRepeatShort->GetFloat();
        if (keyRepeatLong <= 0)
            keyRepeatLong = 0.5f;
        if (keyRepeatShort <= 0)
            keyRepeatShort = 0.3f;
    }

    void LoadINI()
    {
        settingStore->Load();
        SetLogLevel();
        settingStore->Save();
        GetKeyRepeatLongAndShort();
        MCMManager::lock = false;
        checkPrintKey = printNamesKey.GetValue();
    }
}