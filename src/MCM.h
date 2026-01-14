#pragma once

namespace MCMManager
{
    inline bool openMod = false;
    inline bool openPage = false;
    inline bool awaitJournalMenu = false;
    inline bool lock = false;
    inline int pageRetries = 0;
    inline int modRetries = 0;
    inline std::string_view closedMenuName = "None";

    inline std::string configPanel = "_root.ConfigPanelFader.configPanel.";
    inline std::string modListPanel = configPanel + "contentHolder.modListPanel.";
    inline std::string modList = modListPanel + "modListFader.list.";
    inline std::string pageList = modListPanel + "subListFader.list.";
    inline std::string showModList = modListPanel + "showList";

    inline Settings::shortcutInfo currentInfo;

    void OpenFromJournal();

    void CloseOpenMenus();

    void AddUiTask(void (*func)());

    bool IsMCMOpen();
}