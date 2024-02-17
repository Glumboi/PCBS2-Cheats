#pragma once

#define MODNAME "[PCBS2-Cheats]"
#define JSONFILE "\\Plugins\\PCBS2-Cheats.offsetRequests.json"
#define QUICKDEBUG(msg) std::cout << MODNAME << ": " <<  msg << "\n"
#define PRESSED < 0
#define RELEASED > 0

struct CareerStats
{
    Unity::CComponent* owner;

    CareerStats(): owner(nullptr)
    {
    }

    int GetCash()
    {
        return owner->GetMemberValue<int>("m_cash");
    }

    void SetCash(int newValue)
    {
        QUICKDEBUG("Setting cash to " << newValue);
        owner->SetMemberValue<int>("m_cash", newValue);
    }

    int GetKudos()
    {
        return owner->GetMemberValue<int>("m_kudos");
    }

    void SetKudos(int newValue)
    {
        QUICKDEBUG("Setting kudos to " << newValue);
        owner->SetMemberValue<int>("m_kudos", newValue);
    }

    int GetStarRating()
    {
        return owner->GetMemberValue<float>("m_starRating");
    }

    void SetStarRating(float newValue)
    {
        QUICKDEBUG("Setting star rating to " << newValue);
        owner->SetMemberValue<float>("m_starRating", newValue);
    }
};

CareerStats stats;

void KeyboardLoop()
{
    QUICKDEBUG("Starting keyboard loop...");

    while (true)
    {
        Sleep(110);

        if (GetKeyState(VK_LCONTROL) PRESSED && GetKeyState(VK_SHIFT) PRESSED && GetKeyState(VK_OEM_PLUS)
            PRESSED)
        {
            float add = stats.GetStarRating() + 1 > 5 ? 5 : stats.GetStarRating() + 1;
            stats.SetStarRating(add);
            continue;
        }

        if (GetKeyState(VK_LCONTROL) PRESSED && GetKeyState(VK_SHIFT) PRESSED && GetKeyState(
                VK_OEM_MINUS)
            PRESSED)
        {
            const float sub = stats.GetStarRating() - 1 == 0 ? 0 : stats.GetStarRating() - 1;
            stats.SetStarRating(sub);
            continue;
        }

        if (GetKeyState(VK_SHIFT) && GetKeyState(VK_OEM_PLUS) PRESSED)
        {
            stats.SetCash(stats.GetCash() + 1000);
            continue;
        }

        if (GetKeyState(VK_SHIFT)  && GetKeyState(VK_OEM_MINUS) PRESSED)
        {
            stats.SetCash(stats.GetCash() - 1000);
            continue;
        }

        if (GetKeyState(VK_LCONTROL) PRESSED && GetKeyState(VK_OEM_PLUS) PRESSED)
        {
            stats.SetKudos(stats.GetKudos() + 1000);
            continue;
        }

        if (GetKeyState(VK_LCONTROL) PRESSED && GetKeyState(VK_OEM_MINUS) PRESSED)
        {
            const float sub = stats.GetKudos() - 1000 == 0 ? 0 : stats.GetKudos() - 1000;
            stats.SetKudos(sub);
        }
    }
}

void (__fastcall*CareerStatus_State_OnDeserialization_o)(DWORD*, DWORD*, const DWORD*);

void __stdcall CareerStatus_State_OnDeserialization_hook(DWORD* __this, DWORD* sender, const DWORD* method)
{
    //Run original first
    CareerStatus_State_OnDeserialization_o(__this, sender, method);
    Unity::CComponent* caller = (Unity::CComponent*)__this;
    stats.owner = caller;
    QUICKDEBUG("cash: " << stats.GetCash());
    QUICKDEBUG("kudos: " << stats.GetKudos());
    QUICKDEBUG("starRating: " << stats.GetStarRating());

    std::thread keyboardThread(KeyboardLoop);
    keyboardThread.detach();
    MessageBox(NULL, "PCBS2-Cheats has initialized!\n"
               "Control scheme:\n"
               "- shift + '+' to add 1000 cash\n"
               "- shift + '-' to subtract 1000 cash\n"
               "- ctrl + '+' to add 1000 kudos/xp\n"
               "- ctrl + '-' to remove 1000 kudos/xp\n"
               "- ctrl + shift + '+' to add 1 star\n"
               "- ctrl + shift + '+' to subtract 1 star\n", "Info", MB_OK);

    //CreateThread(0, 0, (LPTHREAD_START_ROUTINE)KeyboardLoop, 0, 0, 0);
    return;
}

inline void CreateAndLoadHooks()
{
    //Load addresses from json file
    QUICKDEBUG("Loading offsets from " << JSONFILE);
    //Append json path with dll location
    char buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    std::string dllJsonPath = std::string(buffer) + JSONFILE;

    //Load offsets
    functionOffsets = readJsonFromFile(dllJsonPath).offsetRequests;

    //Print all offsets for debugging, remove when you are done testing your plugin!
    for (auto& offset : functionOffsets)
    {
        QUICKDEBUG("Offset loaded: " << offset.searchName << " at " << offset.value);
    }

    //Instantiate your hooks here using MH_CreateHook
    uintptr_t CareerStatus_State_OnDeserializationOffset = std::stoull(functionOffsets[0].value, nullptr, 16);
    MH_CreateHook(
        reinterpret_cast<LPVOID*>(gameAsm + CareerStatus_State_OnDeserializationOffset),
        &CareerStatus_State_OnDeserialization_hook,
        (LPVOID*)&CareerStatus_State_OnDeserialization_o);


    MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS); //Get the hook status, 0 = ALl good

    QUICKDEBUG("Hooks loaded with status: " << status);
}
