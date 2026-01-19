namespace Events
{
    std::chrono::steady_clock::time_point hotkeyHitTime;

    bool settingsReloaded = false;

    long long NanoToLongMilli(std::chrono::nanoseconds time_point)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point).count();
    }

    // Returns true if the current modifier is down or not set
    bool IsModifierDown(int key, bool IsModifier1, Settings::shortcutInfo shorcut)
    {
        if (key == 0)
            return true;
        else if (IsModifier1 && shorcut.modifier1Down)
            return true;
        else if (!IsModifier1 && shorcut.modifier2Down)
            return true;
        else
            return false;
    }

    // Check if the shortcut is still held and hasn't been released and if it was held past the setting reload threshold.
    bool IsHeldStillAndPastThreshold(bool stillHeld)
    {
        return (stillHeld &&
                NanoToLongMilli(std::chrono::steady_clock::now() - hotkeyHitTime) > Settings::SettingsThreshold.GetValue() &&
                !settingsReloaded);
    }

    // Validate that if the order matters then the keys were hit in the right order
    bool OrderIsOk(Settings::shortcutInfo shortcut)
    {
        if (shortcut.orderMatters)
        {
            if (shortcut.modifier1 != 0)
            {
                if (shortcut.modifier1Order == -1)
                    return false;
                if (shortcut.modifier2 != 0 &&
                    shortcut.modifier2Order != -1 &&
                    shortcut.modifier1Order >= shortcut.modifier2Order)
                    return false;

                if (shortcut.modifier1Order >= shortcut.hotkeyOrder)
                    return false;
            }
            if (shortcut.modifier2 != 0)
            {
                if (shortcut.modifier2Order == -1)
                    return false;
                if (shortcut.modifier2Order >= shortcut.hotkeyOrder)
                    return false;
            }
        }
        return true;
    }

    // Check if the key hit is part of the shortcut & get the order the key was hit in.
    void EvaluateKeyForShortcut(int keyCode, bool isHeld, bool isDown, Settings::shortcutInfo &shortcut)
    {
        if (keyCode == shortcut.hotkey)
        {
            shortcut.hotkeyDown = isHeld || isDown;
            if (isDown)
                shortcut.hotkeyOrder = shortcut.orderIndex++;
        }
        else if (keyCode == shortcut.modifier1)
        {
            shortcut.modifier1Down = isHeld || isDown;
            if (isDown)
                shortcut.modifier1Order = shortcut.orderIndex++;
        }
        else if (keyCode == shortcut.modifier2)
        {
            shortcut.modifier2Down = isHeld || isDown;
            if (isDown)
                shortcut.modifier2Order = shortcut.orderIndex++;
        }
        else
        {
            shortcut.otherKeyHit = true;
            shortcut.otherKeyDown = isHeld || isDown;
        }
    }

    // Check if shortcut has been pressed and on release check if only the shortcut keys were activated.
    bool EvaluateShortcutState(Settings::shortcutInfo &shortcut)
    {
        if (shortcut.hotkeyDown &&
            IsModifierDown(shortcut.modifier1, true, shortcut) &&
            IsModifierDown(shortcut.modifier2, false, shortcut) &&
            !shortcut.otherKeyHit && OrderIsOk(shortcut))
        {
            if (!shortcut.shortcutHit)
            {
                // get hit time for measuring how long shortcut is held for settings reload
                hotkeyHitTime = std::chrono::steady_clock::now();
                shortcut.shortcutHit = true;
                shortcut.stillHeld = true;
            }
            else if (shortcut.canReloadSettings && !shortcut.otherKeyHit && IsHeldStillAndPastThreshold(shortcut.stillHeld))
            {
                settingsReloaded = true;
                Settings::LoadINI();
                Settings::LoadJSON();
                if (Settings::ValidateShortcuts())
                    RE::DebugMessageBox("- MCM Shortcut NG -\nSettings Reloaded");
            }
        }
        else if (!shortcut.hotkeyDown && !shortcut.modifier1Down && !shortcut.modifier2Down && !shortcut.otherKeyDown)
        {
            // Only evaluate on all keys released, shortcut is only valid if there is no other key hit during press
            if (shortcut.shortcutHit && !shortcut.otherKeyHit)
            {
                if (MCMManager::lock)
                {
                    logger::trace("MCM management currently locked due to previous shortcut."sv);
                    return false;
                }
                logger::debug("-           -           -           -           -           -           -           -"sv);
                logger::trace("Shortcut hit"sv);
                MCMManager::currentInfo = shortcut;
                MCMManager::CloseOpenMenus();
            }
            shortcut.shortcutHit = false;
            shortcut.otherKeyHit = false;
            shortcut.stillHeld = false;
            shortcut.hotkeyOrder = -1;
            shortcut.modifier1Order = -1;
            shortcut.modifier2Order = -1;
            shortcut.orderIndex = 0;
        }
        else
        {
            // Prevents settings reloading when you paritally release and rehit a shortcut.
            // I.E. hitting Ctrl+V, releasing and pressing V again will not reload settings
            shortcut.stillHeld = false;
        }
        return true;
    }

    // Loops through all defined shortcuts and evaluates them for the button event
    void CheckShortcuts(RE::ButtonEvent *btn)
    {
        if (btn->IsDown())
            logger::trace("KeyCode: {}"sv, btn->GetIDCode());
        auto device = btn->GetDevice();
        for (Settings::shortcutInfo &shortcut : Settings::shortcutInfos)
        {
            if (device == RE::INPUT_DEVICE::kGamepad && !shortcut.controller || shortcut.controller && device != RE::INPUT_DEVICE::kGamepad)
                continue;

            EvaluateKeyForShortcut(btn->GetIDCode(), btn->IsHeld(), btn->IsDown(), shortcut);

            // If false then MCM is locked by previous hotkey
            if (!EvaluateShortcutState(shortcut))
                return;
        }
        if (settingsReloaded)
        {
            bool noKeysDown = true;
            for (auto shortcut : Settings::shortcutInfos)
            {
                if (shortcut.modifier1Down || shortcut.modifier2Down || shortcut.hotkeyDown || shortcut.otherKeyDown)
                {
                    noKeysDown = false;
                    break;
                }
            }
            if (noKeysDown)
                settingsReloaded = false;
        }
    }

    void CheckIfPrint(RE::ButtonEvent *btn)
    {
        if (!Settings::checkPrintKey || !btn->IsDown() || static_cast<int>(btn->GetIDCode()) != Settings::printNamesKey.GetValue())
            return;
        MCMManager::AddUiTask(MCMManager::PrintNames);
    }

    RE::BSEventNotifyControl InputEvent::ProcessEvent(
        RE::InputEvent *const *a_event,
        RE::BSTEventSource<RE::InputEvent *> *) noexcept
    {
        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }
        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kButton)
            {
                if (auto btn = e->AsButtonEvent(); btn)
                {
                    if (!btn->GetIDCode())
                        return RE::BSEventNotifyControl::kContinue;
                    CheckShortcuts(btn);
                    CheckIfPrint(btn);
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    InputEvent *InputEvent::GetSingleton()
    {
        static InputEvent singleton;
        return &singleton;
    }

    void PrintMenuFlags(RE::IMenu *a_menu, const std::string &a_menuName)
    {
        if (!a_menu)
        {
            logger::trace("PrintMenuFlags: Menu '{}' is null.", a_menuName);
            return;
        }

        using Flag = RE::UI_MENU_FLAGS;

        std::vector<std::pair<Flag, std::string>> flagMap = {
            {Flag::kPausesGame, "kPausesGame (Bit 0)"},
            {Flag::kAlwaysOpen, "kAlwaysOpen (Bit 1)"},
            {Flag::kUsesCursor, "kUsesCursor (Bit 2)"},
            {Flag::kUsesMenuContext, "kUsesMenuContext (Bit 3)"},
            {Flag::kModal, "kModal (Bit 4)"},
            {Flag::kFreezeFrameBackground, "kFreezeFrameBackground (Bit 5)"},
            {Flag::kOnStack, "kOnStack (Bit 6)"},
            {Flag::kDisablePauseMenu, "kDisablePauseMenu (Bit 7)"},
            {Flag::kRequiresUpdate, "kRequiresUpdate (Bit 8)"},
            {Flag::kTopmostRenderedMenu, "kTopmostRenderedMenu (Bit 9)"},
            {Flag::kUpdateUsesCursor, "kUpdateUsesCursor (Bit 10)"},
            {Flag::kAllowSaving, "kAllowSaving (Bit 11)"},
            {Flag::kRendersOffscreenTargets, "kRendersOffscreenTargets (Bit 12)"},
            {Flag::kInventoryItemMenu, "kInventoryItemMenu (Bit 13)"},
            {Flag::kDontHideCursorWhenTopmost, "kDontHideCursorWhenTopmost (Bit 14)"},
            {Flag::kCustomRendering, "kCustomRendering (Bit 15)"},
            {Flag::kAssignCursorToRenderer, "kAssignCursorToRenderer (Bit 16)"},
            {Flag::kApplicationMenu, "kApplicationMenu (Bit 17)"},
            {Flag::kHasButtonBar, "kHasButtonBar (Bit 18)"},
            {Flag::kIsTopButtonBar, "kIsTopButtonBar (Bit 19)"},
            {Flag::kAdvancesUnderPauseMenu, "kAdvancesUnderPauseMenu (Bit 20)"},
            {Flag::kRendersUnderPauseMenu, "kRendersUnderPauseMenu (Bit 21)"},
            {Flag::kUsesBlurredBackground, "kUsesBlurredBackground (Bit 22)"},
            {Flag::kCompanionAppAllowed, "kCompanionAppAllowed (Bit 23)"},
            {Flag::kFreezeFramePause, "kFreezeFramePause (Bit 24)"},
            {Flag::kSkipRenderDuringFreezeFrameScreenshot, "kSkipRenderDuringFreezeFrameScreenshot (Bit 25)"},
            {Flag::kLargeScaleformRenderCacheMode, "kLargeScaleformRenderCacheMode (Bit 26)"},
            {Flag::kUsesMovementToDirection, "kUsesMovementToDirection (Bit 27)"}};

        logger::trace("====== Flags for Menu: {} ======", a_menuName);

        bool anyFound = false;

        for (const auto &[flag, name] : flagMap)
        {
            if ((a_menu->menuFlags & flag) != Flag::kNone)
            {
                logger::trace("  [X] {}", name);
                anyFound = true;
            }
        }

        if (!anyFound)
        {
            logger::trace("  [ ] No flags set (kNone).");
        }
        logger::trace("=========================================");
    }

    RE::BSEventNotifyControl UIEvent::ProcessEvent(
        const RE::MenuOpenCloseEvent *a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent> *)
    {
        if (!a_event || !a_event->opening)
        {
            return RE::BSEventNotifyControl::kContinue;
        }
        RE::UI_MENU_FLAGS::kUsesMovementToDirection;
        auto ui = RE::UI::GetSingleton();
        if (ui)
        {
            auto menu = ui->GetMenu(a_event->menuName);
            if (Settings::log_level.GetValue() == 4)
                PrintMenuFlags(menu.get(), a_event->menuName.c_str());
            if (menu && menu->menuFlags.any(RE::UI_MENU_FLAGS::kUpdateUsesCursor, RE::UI_MENU_FLAGS::kUsesCursor))
            {
                logger::trace("Calling FixKeyRepeat()");
                MCMManager::FixKeyRepeat();
            }
        }

        if (MCMManager::awaitJournalMenu && a_event->menuName == RE::JournalMenu::MENU_NAME)
        {
            MCMManager::awaitJournalMenu = false;
            MCMManager::AddUiTask(MCMManager::OpenFromJournal);
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    UIEvent *UIEvent::GetSingleton()
    {
        static UIEvent singleton;
        return &singleton;
    }
}