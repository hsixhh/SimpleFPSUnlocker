/*Windows*/
#include <Windows.h>
#include <thread>
#include <iostream>
#include <tchar.h>

/*DirectX*/
#include <d3d11.h>

/*ImGui*/
#include "../Resource Files/ocornut/imgui.h"
#include "../Resource Files/ocornut/imgui_impl_dx11.h"
#include "../Resource Files/ocornut/imgui_impl_win32.h"

/*Other*/
#include "offsets.h"
#include "Poppins-SemiBold.c"

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int Height;
int Width;

/*functions*/
ImVec2 GetWindowSize(HWND Window)
{
    RECT rect;
    GetWindowRect(Window, &rect);

    float w = static_cast<float>(rect.right - rect.left);
    float h = static_cast<float>(rect.bottom - rect.top);

    return { w, h };
}

HWND _CreateWindow() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("SFPSU"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, wc.lpszClassName, L"SFPSU", WS_OVERLAPPEDWINDOW, 100, 100, 500, 109, NULL, NULL, wc.hInstance, NULL);
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    RECT rect;
    GetWindowRect(hwnd, &rect);
    Height = rect.bottom - rect.top;;
    Width = rect.right - rect.left;;

    return { hwnd };
}

auto initialize_imgui(HWND wnd) -> void
{
    /*create context to setup*/
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiStyle* style = &ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.FontDefault = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(Data, sizeof(Data), 13.f);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25);
    colors[ImGuiCol_SliderGrab] = ImColor(219, 158, 229);
    colors[ImGuiCol_SliderGrabActive] = ImColor(219, 158, 229);
    colors[ImGuiCol_FrameBgHovered] = ImColor(30, 30, 30);
    colors[ImGuiCol_FrameBgActive] = ImColor(30, 30, 30);
    colors[ImGuiCol_FrameBg] = ImColor(30, 30, 30);

    ImGui_ImplWin32_Init(wnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

auto ImGuiThread() -> void
{
    ImVec4 clear_color = ImColor(30, 30, 30);

    MSG message;
    while (PeekMessage(&message, 0u, 0u, 0u, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(ImVec2(GetWindowSize(FindWindowA(0, "SFPSU")).x, GetWindowSize(FindWindowA(0, "SFPSU")).y));
    ImGui::Begin("Welcome", 0, ImGuiWindowFlags_NoDecoration);

    ImGui::Text("Simple FPS Unlocker | By Nicholas");

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    ImGui::Text("GetTaskScheduler : %x", Globals::Scheduler);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    ImGui::SliderFloat("Framerate Cap", &Globals::FrameCap, 0, 10000);

    ImGui::End();

    ImGui::EndFrame();
    FLOAT color[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(0, NULL);
}

auto Initialize() -> void
{
    HWND hwnd = _CreateWindow();
    initialize_imgui(hwnd);

    while (true)
    {
        ImGuiThread();
    }
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
