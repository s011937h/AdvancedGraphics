#include "Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) //version of the main function
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Application* theApp = new Application(); //creates new application class

    if (FAILED(theApp->Initialise(hInstance, nCmdShow))) //FAILED is a macro, HRESULT is an integer error code
    {
        return -1;
    }

    // Main message loop
    MSG msg = { 0 };

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) //all windows can take messages such as resize window, minimise, etc
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            theApp->Update();
            theApp->Render();
        }
    }

    delete theApp;
    theApp = nullptr;

    return (int)msg.wParam;
}