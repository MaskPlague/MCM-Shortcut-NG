#pragma once

namespace Settings
{
    inline REX::INI::I32 SettingsThreshold{"General", "Settings Reload Threshold Milliseconds", 3000};
    inline REX::INI::Bool CanAlsoCloseMCM{"General", "Can Also Close Opened MCM", true};

    inline REX::INI::Bool CloseDialogue{"Menus", "ActiveInDialogue", true};
    inline REX::INI::Bool CloseInventory{"Menus", "ActiveInInventory", true};
    inline REX::INI::Bool CloseMagic{"Menus", "ActiveInMagic", true};
    inline REX::INI::Bool CloseMap{"Menus", "ActiveInMap", true};

    inline REX::INI::I32 log_level{"Debug", "Log Level", 2};

    inline bool gameHasLoaded = false;

    struct shorcutInfo
    {
        int modifier1 = 0;
        int modifier2 = 0;
        int hotkey = 0;

        bool shortcutHit = false;
        bool stillHeld = false;
        bool otherKeyDown = false;

        bool modifier1Down = false;
        bool modifier2Down = false;
        bool hotkeyDown = false;

        bool openMod = false;
        std::string modName = "None";
        std::string modNameTranslated = "None";
        int modDelay = 50;

        bool openPage = false;
        std::string pageName = "None";
        std::string pageNameTranslated = "None";
        int pageDelay = 500;
    };

    inline std::vector<shorcutInfo> shortcutInfos;

    bool ValidateShortcuts();

    void InitINI();

    void LoadINI();

    void LoadJSON();

}