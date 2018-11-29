#include "ctrlaltdelsimulator.h"

#include "winutilities.h"


namespace HEHUI {


CtrlAltDelSimulator::CtrlAltDelSimulator(QObject *parent)
    :QThread(parent)
{

}

CtrlAltDelSimulator::~CtrlAltDelSimulator()
{

}

void CtrlAltDelSimulator::run()
{
    // Switch thread desktop to "Winlogon".
    if (HEHUI::WinUtilities::selectDesktop("Winlogon")) {
        HWND hwndCtrlAltDel = FindWindowW(L"SAS window class", L"SAS window");
        if (hwndCtrlAltDel == NULL) {
            hwndCtrlAltDel = HWND_BROADCAST;
        }
        PostMessageW(hwndCtrlAltDel, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));
    }
    // Do not restore previous desktop.
}





}
