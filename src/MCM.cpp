namespace MCM
{

    RE::GFxMovieView *GetJournalView()
    {
        auto ui = RE::UI::GetSingleton();
        auto journalMenu = ui->GetMenu(RE::JournalMenu::MENU_NAME);
        return journalMenu ? journalMenu->uiMovie.get() : nullptr;
    }

    void AddUiTask(void (*func)())
    {
        SKSE::GetTaskInterface()->AddUITask([func]()
                                            { func(); });
    }

    void DelayCall(void (*func)(), int delay)
    {
        std::thread([func, delay]()
                    { std::this_thread::sleep_for(std::chrono::milliseconds(delay)); AddUiTask(func); })
            .detach();
    }

    void GetItemIndexFromEntryList(std::string page, const char *varToGet, std::string item)
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
            logger::debug(">  {}: {},   index: {}"sv, varToGet, name.GetString(), i);
            if (item == name.GetString() || (std::strcmp(varToGet, ".modName") == 0 && currentInfo.modNameTranslated == name.GetString()))
            {
                index = i;
                break;
            }
        }
        if (index == -1)
            return;
        RE::GFxValue args[2] = {index, 1};
        view->Invoke((page + "doSetSelectedIndex").c_str(), nullptr, args, 2);
        view->Invoke((page + "onItemPress").c_str(), nullptr, args, 2);
    }

    bool AModIsOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue state;
        std::string _state = root + "_state";
        view->GetVariable(&state, _state.c_str());
        if (!state.IsNumber())
            return false;
        return state.GetNumber() == 2;
    }

    bool IsModAlreadyOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue textVal;
        std::string titleText = root + "_titleText";
        view->GetVariable(&textVal, titleText.c_str());
        if (!textVal.IsString())
            return false;
        return currentInfo.modNameTranslated == textVal.GetString() && AModIsOpen();
    }

    bool APageIsOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;
        RE::GFxValue pageName;
        std::string activePageName = pageList + "listState.activeEntry.pageName";
        view->GetVariable(&pageName, activePageName.c_str());
        if (!pageName.IsString())
            return false;
        return true;
    }

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
        auto view = GetJournalView();
        view->GetVariable(&disabled, (pageList + "disableSelection").c_str());
        if (disabled.IsBool() && disabled.GetBool())
        {
            logger::trace("Selection is disabled for _pageList, recalling with delay"sv);
            pageRetries++;
            DelayCall(OpenPage, currentInfo.pageDelay);
            return;
        }
        logger::debug("Opening page"sv);
        GetItemIndexFromEntryList(pageList, ".pageName", currentInfo.pageName);
        lock = false;
    }

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
        auto view = GetJournalView();
        view->GetVariable(&disabled, (modList + "disableSelection").c_str());
        if (disabled.IsBool() && disabled.GetBool())
        {
            logger::trace("Selection is disabled for _modList, recalling with delay"sv);
            modRetries++;
            DelayCall(OpenMod, currentInfo.modDelay);
            return;
        }
        logger::debug("Opening mod"sv);
        GetItemIndexFromEntryList(modList, ".modName", currentInfo.modName);
        if (currentInfo.openPage)
            DelayCall(OpenPage, currentInfo.pageDelay);
        else
            lock = false;
    }

    bool IsMCMOpen()
    {
        RE::GFxMovieView *view = GetJournalView();
        if (!view)
            return false;

        RE::GFxValue gfxQJFdepth;
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
            return true;
    }

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
        bool aModOpen = AModIsOpen();
        bool pageOpen = IsPageAlreadyOpen();
        bool aPageOpen = APageIsOpen();
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
            logger::trace("Mod/Page already open, closing"sv);
            auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
            uiMessageQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            if (Settings::ReturnToPreviousMenu.GetValue() && closedMenuName != "None")
            {
                logger::debug("Reopening closed menu: {}", closedMenuName);
                uiMessageQueue->AddMessage(closedMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            }
            closedMenuName = "None";
            lock = false;
            return;
        }

        if (!mcmOpen)
        {
            logger::trace("MCM not open, opening"sv);
            view->Invoke("_root.QuestJournalFader.Menu_mc.ConfigPanelOpen", nullptr, nullptr, 0);
        }

        if (!theModOpen && aModOpen)
        {
            logger::trace("The mod is not open but a mod is open, reverting to main MCM"sv);
            RE::GFxValue arg[1] = {4};
            view->Invoke((root + "setState").c_str(), nullptr, arg, 1);
        }

        if (!theModOpen && currentInfo.openMod)
        {
            logger::trace("The mod is not open..."sv);
            DelayCall(OpenMod, currentInfo.modDelay);
            unlock = false;
        }

        if (theModOpen && !pageOpen && currentInfo.openPage)
        {
            logger::trace("The mod is open but the page is not..."sv);
            DelayCall(OpenPage, 0);
            unlock = false;
        }

        if (unlock)
            lock = false;
    }

    void CloseOpenMenus()
    {
        auto ui = RE::UI::GetSingleton();
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
            auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
            if (menuName != "None")
            {
                uiMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                closedMenuName = menuName != RE::DialogueMenu::MENU_NAME ? menuName : "None";
            }
            awaitJournalMenu = true;
            uiMessageQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
        else
            AddUiTask(OpenFromJournal);
    }
}
