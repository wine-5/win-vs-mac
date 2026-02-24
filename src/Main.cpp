#include "DxLib.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    if (DxLib_Init() == -1) return -1;

    WaitKey();

    DxLib_End();
    return 0;
}