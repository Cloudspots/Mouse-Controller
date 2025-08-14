#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>

using namespace std;

enum
{
    HotKey_ShowHelp,
    HotKey_LockCursor,
    HotKey_Clicking,
    HotKey_UDLRControl,
    HotKey_KBU,
    HotKey_KBD,
    HotKey_KBL,
    HotKey_KBR
};

bool CursorLocked = false;
bool Clicking = false;
bool UDLRControl = false;
shared_mutex LockedPosMutex;
POINT LockedPos;

void Do()
{
    while (true)
    {
        if (!CursorLocked && !Clicking) this_thread::yield();
        POINT pt;
        GetCursorPos(&pt);
        if (CursorLocked)
        {
            volatile auto _ = shared_lock<shared_mutex>(LockedPosMutex);
            if(pt.x != LockedPos.x || pt.y != LockedPos.y) SetCursorPos(LockedPos.x, LockedPos.y);
        }
        if (Clicking)
        {
            mouse_event(MOUSEEVENTF_LEFTDOWN, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, pt.x, pt.y, 0, 0);
        }
    }
}

int main()
{
    thread x(Do);
    thread y(Do);
    x.detach();
    RegisterHotKey(NULL, HotKey_ShowHelp, MOD_CONTROL | MOD_ALT, VK_F1);
    RegisterHotKey(NULL, HotKey_LockCursor, MOD_CONTROL | MOD_ALT, VK_F2);
    RegisterHotKey(NULL, HotKey_Clicking, MOD_CONTROL | MOD_ALT, VK_F3);
    RegisterHotKey(NULL, HotKey_UDLRControl, MOD_CONTROL | MOD_ALT, VK_F4);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            switch (msg.wParam)
            {
            case HotKey_ShowHelp:
                MessageBox(NULL, TEXT(R"(Ctrl+Alt+F1：显示此帮助
Ctrl+Alt+2：锁定/解锁鼠标
Ctrl+Alt+3：开始/结束连点
Ctrl+Alt+4：启用/禁用 Ctrl+Shift+键盘上下左右控制鼠标)"), TEXT("帮助"), MB_ICONINFORMATION);
                break;
            case HotKey_LockCursor:
                if (!CursorLocked) GetCursorPos(&LockedPos);
                CursorLocked ^= 1;
                if (!CursorLocked) UDLRControl = false;
                break;
            case HotKey_Clicking:
                Clicking ^= 1;
                break;
            case HotKey_UDLRControl:
                if (!HotKey_LockCursor) break;
                UDLRControl ^= 1;
                if (UDLRControl)
                {
                    RegisterHotKey(NULL, HotKey_KBU, MOD_CONTROL | MOD_SHIFT, VK_UP);
                    RegisterHotKey(NULL, HotKey_KBD, MOD_CONTROL | MOD_SHIFT, VK_DOWN);
                    RegisterHotKey(NULL, HotKey_KBL, MOD_CONTROL | MOD_SHIFT, VK_LEFT);
                    RegisterHotKey(NULL, HotKey_KBR, MOD_CONTROL | MOD_SHIFT, VK_RIGHT);
                }
                else
                {
                    UnregisterHotKey(NULL, HotKey_KBU);
                    UnregisterHotKey(NULL, HotKey_KBD);
                    UnregisterHotKey(NULL, HotKey_KBL);
                    UnregisterHotKey(NULL, HotKey_KBR);
                }
                break;
            case HotKey_KBU:
            {
                unique_lock<shared_mutex> _(LockedPosMutex);
                LockedPos.y -= 5;
                break;
            }
            case HotKey_KBD:
            {
                unique_lock<shared_mutex> _(LockedPosMutex);
                LockedPos.y += 5;
                break;
            }
            case HotKey_KBL:
            {
                unique_lock<shared_mutex> _(LockedPosMutex);
                LockedPos.x -= 5;
                break;
            }
            case HotKey_KBR:
            {
                unique_lock<shared_mutex> _(LockedPosMutex);
                LockedPos.x += 5;
                break;
            }
            }
        }
    }
    return 0;
}