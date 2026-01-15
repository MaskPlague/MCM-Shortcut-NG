#pragma once

namespace Settings
{
    inline REX::INI::I32 SettingsThreshold{"General", "Settings Reload Threshold Milliseconds", 3000};
    inline REX::INI::Bool CanAlsoCloseMCM{"General", "Can Also Close Opened MCM", true};
    inline REX::INI::Bool ReturnToPreviousMenu{"General", "Returns to Previously Closed Menu", true};
    inline REX::INI::Bool NoAutoInputSwitch{"General", "No Auto Input Switch", false};

    inline REX::INI::Bool DisableInDialogueOverride{"Menus", "Disable In Dialogue Override", false};
    inline REX::INI::Bool DisableInInventoryOverride{"Menus", "Disable In Inventory Override", false};
    inline REX::INI::Bool DisableInMagicOverride{"Menus", "Disable In Magic Override", false};
    inline REX::INI::Bool DisableInMapOverride{"Menus", "Disable In Map Override", false};

    inline REX::INI::I32 log_level{"Debug", "Log Level", 2};
    inline REX::INI::I32 printNamesKey{"Debug", "Hotkey To Print Current Menu and Page Name", 0};
    inline bool checkPrintKey = false;

    inline bool gameHasLoaded = false;

    struct shortcutInfo
    {
        int modifier1 = 0;
        int modifier2 = 0;
        int hotkey = 0;

        int orderIndex = 0;
        int modifier1Order = -1;
        int modifier2Order = -1;
        int hotkeyOrder = -1;

        bool orderMatters = false;

        bool controller = false;

        bool canReloadSettings = true;

        bool disableInInventory = false;
        bool disableInDialogue = false;
        bool disableInMagic = false;
        bool disableInMap = false;

        bool shortcutHit = false;
        bool stillHeld = false;
        bool otherKeyDown = false;
        bool otherKeyHit = false;

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

    inline std::vector<shortcutInfo> shortcutInfos;

    bool ValidateShortcuts();

    void InitINI();

    void LoadINI();

    void LoadJSON();

}