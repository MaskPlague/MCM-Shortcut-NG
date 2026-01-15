namespace Input
{
    void PressKey()
    {
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wVk = 0;
        ip.ki.wScan = scanCode;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &ip, sizeof(INPUT));
    }

    void ReleaseKey()
    {
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wVk = 0;
        ip.ki.wScan = scanCode;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }

    void BackupJournalOpenMethod()
    {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (!uiMessageQueue)
            return;
        uiMessageQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }

    uint32_t GetMappedScanCode(RE::ControlMap *controlMap, const RE::BSFixedString &a_event, RE::INPUT_DEVICE device)
    {
        return controlMap->GetMappedKey(a_event, device, RE::UserEvents::INPUT_CONTEXT_ID::kGameplay);
    }

    uint32_t GetKeyToPress(RE::INPUT_DEVICE device)
    {
        RE::UserEvents *userEvents = RE::UserEvents::GetSingleton();
        RE::ControlMap *controlMap = RE::ControlMap::GetSingleton();
        if (!controlMap || !userEvents)
            return 0;

        uint32_t code = GetMappedScanCode(controlMap, userEvents->pause, device);
        if (code != 0 && code != 0xFF)
            return code;

        code = GetMappedScanCode(controlMap, userEvents->journal, device);
        if (code != 0 && code != 0xFF)
            return code;

        return 0;
    }

    void OpenViaKeyboard()
    {
        scanCode = static_cast<WORD>(GetKeyToPress(RE::INPUT_DEVICE::kKeyboard));

        if (scanCode != 0)
        {
            MCMManager::DelayCallForUI(PressKey, 10);
            MCMManager::DelayCallForUI(ReleaseKey, 30);
        }
        else
            BackupJournalOpenMethod();
    }

    void OpenJournalMenu()
    {
        bool isGamepad = RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled();
        if (isGamepad && Settings::NoAutoInputSwitch.GetValue())
        {
            // I can't figure out how to send button inputs without crashing so I'm just going to require AutoInputSwitch.
            // If people don't want to or can't use Auto Input Switch then we use the buggy way.
            BackupJournalOpenMethod();
        }
        else
            OpenViaKeyboard();
    }
}