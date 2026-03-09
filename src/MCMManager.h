#pragma once

namespace MCMManager
{
    // inline bool openMod = false;
    // inline bool openPage = false;
    inline bool awaitJournalMenu = false;
    inline bool escTabClose = true;
    inline bool lock = false;
    inline int pageRetries = 0;
    inline int modRetries = 0;
    inline std::string_view closedMenuName = "None";

    inline std::string pageName;
    inline std::string pageNameTranslated;
    inline std::string modName;
    inline std::string modNameTranslated;
    inline int pageDelay = 0;
    inline int modDelay = 0;
    inline bool openMod = false;
    inline bool openPage = false;

    inline std::string configPanel = "_root.ConfigPanelFader.configPanel.";
    inline std::string modListPanel = configPanel + "contentHolder.modListPanel.";
    inline std::string modList = modListPanel + "modListFader.list.";
    inline std::string pageList = modListPanel + "subListFader.list.";
    inline std::string showModList = modListPanel + "showList";

    inline Settings::shortcutInfo *currentInfo = nullptr;

    void PrintNames();

    void OpenFromJournal();

    void CloseOpenMenus();

    void AddUiTask(void (*func)());

    void DelayCallForUI(void (*func)(), int delay);

    void DelayCallForTask(void (*func)(), int delay);

    bool IsMCMOpen();

    void FixKeyRepeat();
}