namespace MCMManager
{
    // Returns the journal view
    RE::GFxMovieView *GetJournalView()
    {
        auto ui = RE::UI::GetSingleton();
        auto journalMenu = ui->GetMenu(RE::JournalMenu::MENU_NAME);
        return journalMenu ? journalMenu->uiMovie.get() : nullptr;
    }

    void FixKeyRepeat()
    {
        auto menuControls = RE::MenuControls::GetSingleton();
        if (!menuControls)
            return;

        menuControls->directionHandler->keyRepeatLong = Settings::keyRepeatLong;   // Set these because sometimes they unset.
        menuControls->directionHandler->keyRepeatShort = Settings::keyRepeatShort; // Setting these fixes the fast scroll bug
    }

    // Adds a UI Task to the SKSE task interface
    void AddUiTask(void (*func)())
    {
        SKSE::GetTaskInterface()->AddUITask([func]()
                                            { func(); });
    }

    // Delays a function call by delay (milliseconds)
    void DelayCallForUI(void (*func)(), int delay)
    {
        std::thread([func, delay]()
                    { std::this_thread::sleep_for(std::chrono::milliseconds(delay)); AddUiTask(func); })
            .detach();
    }

    // Old method
    // Gets the index of the item in the entry list of the page and selects and clicks it
    /*void GetItemIndexFromEntryList(std::string page, const char *varToGet, std::string item)
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return;
        RE::GFxValue length;
        std::string entryList = page + "_entryList.";
        view->GetVariable(&length, (entryList + "length").c_str());
        if (!length.IsNumber())
            return;
        if (length.GetNumber() == 0)
            logger::debug("No entries to check for {}s, consider increasing delay"sv, varToGet, length.GetNumber());
        else
            logger::debug("Number of entries to check for {}s: {}"sv, varToGet, length.GetNumber());
        int index = -1;
        for (int i = 0; i < length.GetNumber(); i++)
        {
            RE::GFxValue name;
            view->GetVariable(&name, (entryList + std::to_string(i) + varToGet).c_str());
            if (!name.IsString())
                continue;
            const char *nameStr = name.GetString();
            logger::debug(">  {}: {},   index: {}"sv, varToGet, nameStr, i);
            if (item == nameStr || (std::strcmp(varToGet, ".modName") == 0 && currentInfo.modNameTranslated == nameStr))
            {
                index = i;
                break;
            }
        }
        if (index == -1)
            return;
        RE::GFxValue args[2] = {index, 0};
        view->Invoke((page + "doSetSelectedIndex").c_str(), nullptr, args, 2);
        view->Invoke((page + "onItemPress").c_str(), nullptr, args, 2);
    }*/

    // Gets the index of the item in the entry list of the page and selects and clicks it
    void GetItemIndexFromEntryList(std::string pagePath, const char *varToGet, std::string item)
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return;
        RE::GFxValue pageObj;
        view->GetVariable(&pageObj, pagePath.c_str());
        if (!pageObj.IsObject())
            return;
        RE::GFxValue entryList;
        pageObj.GetMember("_entryList", &entryList);
        if (!entryList.IsArray())
            return;
        uint32_t length = entryList.GetArraySize();
        if (length == 0)
        {
            logger::debug("No entries to check for {}s, consider increasing delay"sv, varToGet, length);
            return;
        }
        logger::debug("Number of entries to check for {}s: {}"sv, varToGet, length);
        int index = -1;
        for (uint32_t i = 0; i < length; i++)
        {
            RE::GFxValue entryObject;
            entryList.GetElement(i, &entryObject);
            RE::GFxValue nameVal;
            entryObject.GetMember(varToGet, &nameVal);
            if (!nameVal.IsString())
                continue;
            const char *nameStr = nameVal.GetString();
            logger::debug(">  {}: {},   index: {}"sv, varToGet, nameStr, i);
            if (item == nameStr || (std::strcmp(varToGet, "modName") == 0 && currentInfo.modNameTranslated == nameStr))
            {
                index = i;
                break;
            }
        }
        if (index == -1)
            return;
        RE::GFxValue args[2] = {index, 0};
        pageObj.Invoke("doSetSelectedIndex", nullptr, args, 2);
        pageObj.Invoke("onItemPress", nullptr, args, 2);
    }

    // Is any mod open in the MCM
    bool IsAnyModOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue state;
        std::string _state = modListPanel + "_state";
        view->GetVariable(&state, _state.c_str());
        if (!state.IsNumber())
            return false;
        return state.GetNumber() == 2;
    }

    // Is the currentInfo mod open in the MCM
    bool IsModAlreadyOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue textVal;
        std::string titleText = modListPanel + "_titleText";
        view->GetVariable(&textVal, titleText.c_str());
        if (!textVal.IsString())
            return false;
        return currentInfo.modNameTranslated == textVal.GetString() && IsAnyModOpen();
    }

    // Is any mod's page open in the MCM, if there are any entries in the optionsList
    bool IsAnyPageOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue entryList;
        std::string _entryList = configPanel + "contentHolder.optionsPanel.optionsList._entryList";
        view->GetVariable(&entryList, _entryList.c_str());
        if (!entryList.IsArray())
            return false;
        return entryList.GetArraySize() > 0;

        // Left just in case, not really accurate as the activeEntry is kept until a new MCM mod is loaded
        /*RE::GFxValue pageName;
        std::string activePageName = pageList + "listState.activeEntry.pageName";
        view->GetVariable(&pageName, activePageName.c_str());
        if (!pageName.IsString())
            return false;
        return true;*/
    }

    // Is the currentInfo page open in the MCM
    bool IsPageAlreadyOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue pageName;
        std::string activePageName = pageList + "listState.activeEntry.pageName";
        view->GetVariable(&pageName, activePageName.c_str());
        if (!pageName.IsString())
            return false;
        return currentInfo.pageName == pageName.GetString();
    }

    void PrintNames()
    {
        if (!IsMCMOpen())
            return;
        if (!IsAnyModOpen())
        {
            RE::DebugMessageBox("- MCM Shortcut NG -\nNo mod currently open. Set the Mod name to \"None\" in order to open this page.");
            return;
        }
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return;
        std::string modNameStr = "Could Not Be Found";
        std::string pageNameStr = "Could Not Be Found";
        std::string message = "- MCM Shortcut NG -\nMod Name: ";
        RE::GFxValue modName;
        view->GetVariable(&modName, (modListPanel + "_titleText").c_str());
        if (modName.IsString())
            modNameStr = modName.GetString();
        message = message + modNameStr;
        if (IsAnyPageOpen())
        {
            RE::GFxValue pageName;
            view->GetVariable(&pageName, (pageList + "listState.activeEntry.pageName").c_str());
            if (pageName.IsString())
                pageNameStr = pageName.GetString();
            message = message + "\nPage Name: " + pageNameStr;
        }

        RE::DebugMessageBox(message.c_str());
    }

    // Opens the currentInfo page, will retry itself 20 times if selection is disabled for pageList
    void OpenPage()
    {
        if (pageRetries >= 20)
        {
            logger::trace("Page retry timeout"sv);
            pageRetries = 0;
            lock = false;
            return;
        }
        RE::GFxValue disabled;
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return;
        view->GetVariable(&disabled, (pageList + "disableSelection").c_str());
        if (disabled.IsBool() && disabled.GetBool())
        {
            logger::trace("Selection is disabled for _pageList, recalling with delay"sv);
            pageRetries++;
            DelayCallForUI(OpenPage, currentInfo.pageDelay);
            return;
        }
        logger::debug("Opening page"sv);
        GetItemIndexFromEntryList(pageList, "pageName", currentInfo.pageName);
        lock = false;
    }

    // Opens the currentInfo mod, will retry itself 20 times if selection is disabled for modList
    void OpenMod()
    {
        if (modRetries >= 20)
        {
            logger::trace("Mod retry timeout"sv);
            modRetries = 0;
            lock = false;
            return;
        }
        RE::GFxValue disabled;
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return;
        view->GetVariable(&disabled, (modList + "disableSelection").c_str());
        if (disabled.IsBool() && disabled.GetBool())
        {
            logger::trace("Selection is disabled for _modList, recalling with delay"sv);
            modRetries++;
            DelayCallForUI(OpenMod, currentInfo.modDelay);
            return;
        }
        logger::debug("Opening mod"sv);
        GetItemIndexFromEntryList(modList, "modName", currentInfo.modName);
        if (currentInfo.openPage)
            DelayCallForUI(OpenPage, currentInfo.pageDelay);
        else
            lock = false;
    }

    // Is the MCM currently open, checks the config panel's alpha
    bool IsMCMOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue alpha;
        view->GetVariable(&alpha, (configPanel + "_alpha").c_str());
        if (!alpha.IsNumber())
            return false;
        return alpha.GetNumber() == 100;
        // Leaving this here just in case alpha ends up being unreliable.
        // Is the MCM currently open, checks via comparing the Quest Journal and Config Panel depths
        /*RE::GFxValue gfxQJFdepth;
         double qjfDepth = -1.0;
         RE::GFxValue gfxCPFdepth;
         double cpfDepth = -1.0;

         if (Settings::CanAlsoCloseMCM.GetValue())
         {
             view->Invoke("_root.QuestJournalFader.getDepth", &gfxQJFdepth, nullptr, 0);
             if (gfxQJFdepth.IsNumber())
                 qjfDepth = gfxQJFdepth.GetNumber();
             view->Invoke("_root.ConfigPanelFader.getDepth", &gfxCPFdepth, nullptr, 0);
             if (gfxCPFdepth.IsNumber())
                 cpfDepth = gfxCPFdepth.GetNumber();
         }

         if (cpfDepth < qjfDepth || !Settings::CanAlsoCloseMCM.GetValue())
             return false;
         else
             return true;*/
    }

    // Open MCM/Mod/Page or close the MCM if the MCM or mod or page that was to open is already open, if allowed.
    void OpenFromJournal()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
        {
            lock = false;
            return;
        }
        RE::GFxValue args[2] = {2, false};
        view->Invoke("_root.QuestJournalFader.Menu_mc.RestoreSavedSettings", nullptr, args, 2);
        modRetries = 0;
        pageRetries = 0;
        bool mcmOpen = IsMCMOpen();
        bool theModOpen = IsModAlreadyOpen();
        bool aModOpen = IsAnyModOpen();
        bool pageOpen = IsPageAlreadyOpen();
        bool aPageOpen = IsAnyPageOpen();
        bool unlock = true;
        /*logger::trace("MCM is Open:      {}"sv, mcmOpen);
        logger::trace("The Mod is open:  {},    to be open:    {}"sv, theModOpen, currentInfo.openMod);
        logger::trace("A Mod is open:    {}"sv, aModOpen);
        logger::trace("The Page is open: {},    to be open:    {}"sv, pageOpen, currentInfo.openPage);
        logger::trace("A Page is open:   {}"sv, aPageOpen);*/
        if (mcmOpen && Settings::CanAlsoCloseMCM.GetValue() &&
            ((theModOpen && currentInfo.openMod && pageOpen && currentInfo.openPage) ||
             (theModOpen && currentInfo.openMod && !aPageOpen && !currentInfo.openPage) ||
             (theModOpen && !currentInfo.openMod) ||
             (!aModOpen && !currentInfo.openMod)))
        {
            logger::debug("Mod/Page already open, closing"sv);
            auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
            uiMessageQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            if (Settings::ReturnToPreviousMenu.GetValue() && closedMenuName != "None")
            {
                logger::debug("Reopening closed menu: {}", closedMenuName);
                reopeningClosedMenu = true;
                uiMessageQueue->AddMessage(closedMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            }
            lock = false;
            return;
        }

        if (!mcmOpen)
        {
            logger::debug("MCM not open, opening"sv);
            view->Invoke("_root.QuestJournalFader.Menu_mc.ConfigPanelOpen", nullptr, nullptr, 0);
        }

        if (!theModOpen && aModOpen)
        {
            logger::debug("The mod is not open but a mod is open, reverting to main MCM"sv);
            RE::GFxValue arg[1] = {4};
            // setState(TRANSITION_TO_LIST)
            view->Invoke((modListPanel + "setState").c_str(), nullptr, arg, 1);
        }

        if (!theModOpen && currentInfo.openMod)
        {
            logger::debug("The mod is not open..."sv);
            DelayCallForUI(OpenMod, currentInfo.modDelay);
            unlock = false;
        }

        if (theModOpen && !pageOpen && currentInfo.openPage)
        {
            logger::debug("The mod is open but the page is not..."sv);
            DelayCallForUI(OpenPage, 0);
            unlock = false;
        }

        if (unlock)
            lock = false;
    }

    void OpenMCM()
    {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (!uiMessageQueue)
        {
            lock = false;
            return;
        }
        FixKeyRepeat();
        uiMessageQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }

    // Close the currently opened menu if allowed, open the journal if need be, and then call OpenFromJournal
    void CloseOpenMenus()
    {
        auto ui = RE::UI::GetSingleton();
        if (!ui)
            return;
        std::string_view menuName = "None";

        if (ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME))
            if (Settings::DisableInInventoryOverride.GetValue() || currentInfo.disableInInventory)
                return;
            else
                menuName = RE::InventoryMenu::MENU_NAME;

        if (ui->IsMenuOpen(RE::MapMenu::MENU_NAME))
            if (Settings::DisableInMapOverride.GetValue() || currentInfo.disableInMap)
                return;
            else
                menuName = RE::MapMenu::MENU_NAME;

        if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME))
            if (Settings::DisableInDialogueOverride.GetValue() || currentInfo.disableInDialogue)
                return;
            else
                menuName = RE::DialogueMenu::MENU_NAME;

        if (ui->IsMenuOpen(RE::MagicMenu::MENU_NAME))
            if (Settings::DisableInMagicOverride.GetValue() || currentInfo.disableInMagic)
                return;
            else
                menuName = RE::MagicMenu::MENU_NAME;

        if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME))
            return;

        if (ui->IsMenuOpen(RE::JournalMenu::MENU_NAME))
            menuName = RE::JournalMenu::MENU_NAME;

        lock = true;
        if (menuName != RE::JournalMenu::MENU_NAME)
        {
            if (menuName != "None")
            {
                auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
                if (!uiMessageQueue)
                {
                    logger::trace("Could not get uiMessageQueu to close menu.");
                    lock = false;
                    return;
                }
                uiMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                closedMenuName = menuName != RE::DialogueMenu::MENU_NAME ? menuName : "None";
            }
            awaitJournalMenu = true;
            OpenMCM();
        }
        else
            AddUiTask(OpenFromJournal);
    }
}
