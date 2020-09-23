#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

using namespace std;

uintptr_t moduleAddr;
DWORD processId;
HWND hwnd;
HANDLE processHandle;

struct offests
{
    const DWORD dwLocalPlayer = 0xD3AC5C;
    const DWORD dwGlowObjectManager = 0x5296FF0;
    const DWORD dwEntityList = 0x4D4F1FC;
    const DWORD m_iTeamNum = 0xF4;
    const DWORD m_iGlowIndex  = 0xA438;
    const DWORD m_iHealth = 0x100;
    const DWORD m_bDormant = 0xED;
    const DWORD m_ArmorValue = 0xB378;
    const DWORD m_bSpotted = 0x93D;
}offsets;

template<typename T>RPM(SIZE_T address) {
    T buffer;
    ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template<typename T> void WPM(SIZE_T address, T buffer) {
    WriteProcessMemory(processHandle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

uintptr_t getModuleBaseAdress(const char *);

uintptr_t getLocalPlayer();

void trackRadar();

void esp();

void trackEnemyHealth();

int main()
{
    hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
    GetWindowThreadProcessId(hwnd, &processId);
    moduleAddr = getModuleBaseAdress("client.dll");
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
    while(true) {
        if(GetAsyncKeyState(VK_INSERT)) {
            while(!GetAsyncKeyState(VK_INSERT)) {
                esp();
            }
        }
        else if(GetAsyncKeyState(VK_HOME)) {
            while(!GetAsyncKeyState(VK_HOME)) {
                trackEnemyHealth();
            }
        }
        else if(GetAsyncKeyState(VK_DELETE)) {
            while(!GetAsyncKeyState(VK_DELETE)) {
                trackRadar();
            }
        }
        else if(GetAsyncKeyState(VK_END)) break;
    }
    CloseHandle(processHandle);
    return 0;
}

uintptr_t getModuleBaseAdress(const char *moduleName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if(hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if(Module32First(hSnap, &modEntry)) {
            do {
                if(!strcmp(modEntry.szModule, moduleName)) {
                    CloseHandle(hSnap);
                    return (uintptr_t)modEntry.modBaseAddr;
                }

            } while(Module32Next(hSnap, &modEntry));
        }
    }
}

uintptr_t getLocalPlayer()
{
    return RPM<uintptr_t>(moduleAddr + offsets.dwLocalPlayer);
}

void trackRadar()
{
    for(__int8 i = 1; i < 64; i++) {
        DWORD dwCurrentEntity = RPM<DWORD>(moduleAddr + offsets.dwEntityList + i * 0x10);
        if(dwCurrentEntity) {
            WPM<bool>(dwCurrentEntity + offsets.m_bSpotted, true);
        }
    }
}

void esp()
{
    __int8 myTeam =  RPM<__int8>(getLocalPlayer() + offsets.m_iTeamNum);
    if(!getLocalPlayer()) {
        while(!getLocalPlayer()) { }
    }
    DWORD glowObject = RPM<DWORD>(moduleAddr + offsets.dwGlowObjectManager);
    for(__int8 i = 0; i < 64; i++) {
        DWORD entity = RPM<DWORD>(moduleAddr + offsets.dwEntityList + i * 0x10);
        if(entity) {
            __int8 entityTeam = RPM<__int8>(entity + offsets.m_iTeamNum);
            __int8 glowIndex = RPM<__int8>(entity + offsets.m_iGlowIndex);
            __int8 playerHp = RPM<__int8>(entity + offsets.m_iHealth);
            if(myTeam != entityTeam) {
                if(playerHp <= 100 && playerHp >= 66) {
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x4), 0);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x8), 2);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0xC), 0);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x10), 1.7);
                } else if (playerHp <= 66 && playerHp >= 33) {
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x4), 2);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x8), 2);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0xC), 0);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x10), 1.7);
                } else {
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x4), 2);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x8), 0);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0xC), 0);
                    WPM<float>(glowObject + ((glowIndex * 0x38) + 0x10), 1.7);
                }
            } else continue;
                WPM<bool>(glowObject + ((glowIndex * 0x38) + 0x24), true);
                WPM<bool>(glowObject + ((glowIndex* 0x38) + 0x25), false);
        }
    }
}

void trackEnemyHealth()
{
    __int8 myTeamNum = RPM<__int8>(getLocalPlayer() + offsets.m_iTeamNum);
    for(__int8 i = 0; i < 10; i++) {
        uintptr_t dwEntity = RPM<uintptr_t>(moduleAddr + offsets.dwEntityList + i * 0x10);
        __int8 playerHp = RPM<__int8>(dwEntity + offsets.m_iHealth);
        __int8 playerArmor = RPM<__int8>(dwEntity + offsets.m_ArmorValue);
        __int8 playerTeamNum = RPM<__int8>(dwEntity + offsets.m_iTeamNum);
        if(myTeamNum != playerTeamNum) {
            cout << "Current HP: " << (short int) playerHp << " current armor: " << (short int) playerArmor << endl;
        }
    }
    Sleep(100);
    system("cls");
}
