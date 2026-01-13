namespace Events
{
    std::chrono::steady_clock::time_point hotkeyHitTime;

    /*bool modifier1Down = false;
    bool modifier2Down = false;
    bool hotkeyDown = false;*/
    bool settingsReloaded = false;

    long long NanoToLongMilli(std::chrono::nanoseconds time_point)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point).count();
    }

    bool IsModifierDown(int key, bool IsModifier1, Settings::shorcutInfo shorcut)
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

    bool IsHeldStillAndPastThreshold(bool stillHeld)
    {
        return (stillHeld &&
                NanoToLongMilli(std::chrono::steady_clock::now() - hotkeyHitTime) > Settings::SettingsThreshold.GetValue() &&
                !settingsReloaded);
    }

    void CheckHotkeys(RE::ButtonEvent *btn)
    {
        int keyCode = btn->GetIDCode();
        if (btn->IsDown())
            logger::trace("KeyCode: {}"sv, keyCode);
        for (Settings::shorcutInfo &shortcut : Settings::shortcutInfos)
        {
            if (keyCode == shortcut.hotkey)
                shortcut.hotkeyDown = btn->IsHeld() || btn->IsDown();
            else if (keyCode == shortcut.modifier1)
                shortcut.modifier1Down = btn->IsHeld() || btn->IsDown();
            else if (keyCode == shortcut.modifier2)
                shortcut.modifier2Down = btn->IsHeld() || btn->IsDown();
            else
                shortcut.otherKeyDown = btn->IsHeld() || btn->IsDown();

            if (shortcut.hotkeyDown &&
                IsModifierDown(shortcut.modifier1, true, shortcut) &&
                IsModifierDown(shortcut.modifier2, false, shortcut) &&
                !shortcut.otherKeyDown)
            {
                if (!shortcut.shortcutHit)
                {
                    if (MCM::lock)
                    {
                        logger::trace("MCM Locked");
                        return;
                    }
                    logger::trace("Shortcut hit"sv);
                    shortcut.shortcutHit = true;
                    shortcut.stillHeld = true;
                    hotkeyHitTime = std::chrono::steady_clock::now();
                    MCM::currentInfo = shortcut;
                    MCM::CloseOpenMenus();
                }
                else if (IsHeldStillAndPastThreshold(shortcut.stillHeld))
                {
                    settingsReloaded = true;
                    Settings::LoadINI();
                    Settings::LoadJSON();
                    if (Settings::ValidateShortcuts())
                        RE::DebugMessageBox("MCM Hotkey NG Settings Reloaded");
                }
            }
            else if (!shortcut.hotkeyDown && !shortcut.modifier1Down && !shortcut.modifier2Down)
            {
                shortcut.shortcutHit = false;
            }
            else
            {
                shortcut.stillHeld = false;
            }
        }
        if (settingsReloaded)
        {
            bool noKeysDown = true;
            for (auto shortcut : Settings::shortcutInfos)
            {
                if (shortcut.modifier1Down || shortcut.modifier2Down || shortcut.hotkeyDown)
                {
                    noKeysDown = false;
                    break;
                }
            }
            if (noKeysDown)
                settingsReloaded = false;
        }
        // logger::info("Locked?: {}", MCM::lock);
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
                    auto device = e->GetDevice();
                    if (device == RE::INPUT_DEVICE::kGamepad)
                        return RE::BSEventNotifyControl::kContinue;

                    CheckHotkeys(btn);
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

    RE::BSEventNotifyControl UIEvent::ProcessEvent(
        const RE::MenuOpenCloseEvent *a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent> *)
    {
        if (!a_event || !a_event->opening || a_event->menuName != RE::JournalMenu::MENU_NAME || !MCM::awaitJournalMenu)
        {
            return RE::BSEventNotifyControl::kContinue;
        }
        MCM::awaitJournalMenu = false;
        MCM::AddUiTask(MCM::OpenFromJournal);
        return RE::BSEventNotifyControl::kContinue;
    }

    UIEvent *UIEvent::GetSingleton()
    {
        static UIEvent singleton;
        return &singleton;
    }
}