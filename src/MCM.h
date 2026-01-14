#pragma once

namespace MCM
{
    inline bool openMod = false;
    inline bool openPage = false;
    inline bool awaitJournalMenu = false;
    inline bool lock = false;
    inline int pageRetries = 0;
    inline int modRetries = 0;

    inline std::string root = "_root.ConfigPanelFader.configPanel.contentHolder.modListPanel.";
    inline std::string modList = root + "modListFader.list.";
    inline std::string pageList = root + "subListFader.list.";
    inline std::string showModList = root + "showList";

    inline Settings::shorcutInfo currentInfo;

    void OpenFromJournal();

    void CloseOpenMenus();

    void AddUiTask(void (*func)());

    bool IsMCMOpen();
}