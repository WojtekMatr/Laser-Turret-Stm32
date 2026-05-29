// Dear ImGui: standalone example application for Windows API + DirectX 11

// Learn 
// :
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <windows.h>
#include <string>
#include <cmath>
#include <vector>
#include "XamppApp.h"
#include <algorithm>


struct RadarTarget {
    float distance; 
    float angle; 
};
struct WpisBazy {
    const char* data;
    const char* godzina;
    const char* tryb;
    const char* ruch;
    const RadarTarget* obiekt;
};
// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;


int vImpulsX = 1500;
int vImpulsY = 1500;

HANDLE hSerial;
std::string ostatniaKomenda = "Brak";
int trybPracy = 0;
int serwoMechanizm = 0;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void WyslijKomende(char key, const std::string& nazwaAkcji) {
    if (hSerial != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        WriteFile(hSerial, &key, 1, &bytes_written, NULL);
        ostatniaKomenda = nazwaAkcji + " (" + std::string(1, key) + ")";
      
        if (key == 'w' || key == 'W') vImpulsY += 50;
        else if (key == 's' || key == 'S') vImpulsY -= 50;
        else if (key == 'a' || key == 'A') vImpulsX -= 50;
        else if (key == 'd' || key == 'D') vImpulsX += 50;
        else if (key == ' ') {
            vImpulsX = 1500;
            vImpulsY = 1500;
        }

        if (vImpulsX > 2500) vImpulsX = 2500;
        if (vImpulsX < 500) vImpulsX = 500;
        if (vImpulsY > 2000) vImpulsY = 2000;
        if (vImpulsY < 1000) vImpulsY = 1000;
        if (key == 'f' || key == 'F') {
            baza.ZapiszStrzal();
        }
        else if (key == 'w' || key == 's' || key == 'a' || key == 'd' ||
            key == 'W' || key == 'S' || key == 'A' || key == 'D' || key == ' ') {
            baza.ZapiszRuch(trybPracy, vImpulsX, vImpulsY);
        }
    }
}


bool InicjalizujPortCOM(const std::string& portName) {
    hSerial = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) return false;

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) return false;

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) return false;
    return true;
}
void RysujRadar() {
    static float max_range = 5000.0f;
    static float current_range = 1000.0f;
    static float sweep_speed_rpm = 5.0f;
    static float sweep_angle = 0.0f;

    ImGui::Begin("Skaner poziomy");
    //ImGui::SliderFloat("Zasieg (m)", &current_range, 100.0f, max_range, "%.0f");
    ImGui::Separator();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 region_avail = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    //float radius = std::min(region_avail.x, region_avail.y) * 0.5f;
    //ImVec2 center = ImVec2(window_pos.x + region_avail.x * 0.5f, window_pos.y + radius);




    

    ImGui::End();
}

void RysujInterfejsWiezyczki() {




    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);



    ImGui::Begin("Stacja Dowodzenia Stm32", nullptr, ImGuiWindowFlags_NoCollapse);



    ImGui::Text("Port COM:"); ImGui::SameLine();

    if (hSerial != INVALID_HANDLE_VALUE) {

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Polaczono)");

    }

    else {

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Blad Polaczenia!");

    }

    ImGui::Separator();


    ImGui::Text("Tryb pracy:");

    if (ImGui::RadioButton("Manualny WSAD", &trybPracy, 0)) {
        WyslijKomende('0', "Tryb: Manualny");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Automatyczny", &trybPracy, 1)) {
        WyslijKomende('1', "Tryb: Auto (Kamera)");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Skanowanie terenu", &trybPracy, 2)) {
        WyslijKomende('2', "Tryb: Skanowanie (Radar)");
    }



    ImGui::Text("Typ serwomechnizmow:");
    




    ImGui::RadioButton("Serwomechaznim z przekladnia", &serwoMechanizm, 0); ImGui::SameLine();
    


    ImGui::RadioButton("Serwomechaznim ciaglej pracy", &serwoMechanizm, 1);



    ImGui::Separator();



    ImGui::Text("Sterowanie:");

    float button_width = 100;
    float button_height = 40;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + button_width + spacing);
    if (ImGui::Button("W", ImVec2(button_width, button_height)) || ImGui::IsKeyPressed(ImGuiKey_W, true)) {

        WyslijKomende('w', "Gora");

    }
    if (ImGui::Button("A", ImVec2(100, 40)) || ImGui::IsKeyPressed(ImGuiKey_A, true)) {

        WyslijKomende('a', "Lewo");

    }

    ImGui::SameLine();

    if (ImGui::Button("S", ImVec2(button_width, button_height)) || ImGui::IsKeyPressed(ImGuiKey_S, true)) {

        WyslijKomende('s', "Dol");

    }
    ImGui::SameLine();

    if (ImGui::Button("D", ImVec2(100, 40)) || ImGui::IsKeyPressed(ImGuiKey_D, true)) {

        WyslijKomende('d', "Prawo");

    }


    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

    if (ImGui::Button("F - strzal", ImVec2(-1, 40)) || ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        WyslijKomende('f', "Strzal Laserem");
    }

    ImGui::PopStyleColor(2);
        

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));

    if (ImGui::Button("Wymus zatrzymanie - spacja", ImVec2(-1, 50)) || ImGui::IsKeyPressed(ImGuiKey_Space, true)) {

        WyslijKomende(' ', "STOP AWARYJNY");

    }

    ImGui::PopStyleColor(2);

    ImGui::Separator();

    ImGui::Text("Wyslano: "); ImGui::SameLine();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", ostatniaKomenda.c_str());

    ImGui::End();
}
void RysujBazeDanych() {
    ImGui::SetNextWindowSize(ImVec2(650, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Panel Kontroli Bazy Danych", nullptr, ImGuiWindowFlags_NoCollapse);

    // Status po³¹czenia i przycisk odœwie¿ania
    ImGui::Text("Status MySQL: "); ImGui::SameLine();
    if (baza.getPolaczono()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "POLACZONO");
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DISCONNECTED");
    }

    if (ImGui::Button("POBIERZ / ODSWIEZ DANE Z XAMPPA", ImVec2(-1, 35))) {
        baza.PobierzDaneDoZakladek();
    }
    ImGui::Separator();

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTabBar("PasekZakladek")) {

        // --- ZAK£ADKA: PO£¥CZENIE ---
        if (ImGui::BeginTabItem("Polaczenie")) {
            if (ImGui::BeginTable("PolaczenieTable", 3, flags, ImVec2(0, 300))) {
                ImGui::TableSetupColumn("ID Polaczenia");
                ImGui::TableSetupColumn("Polaczenie");
                ImGui::TableSetupColumn("Zerwanie");
                ImGui::TableHeadersRow();

                // Pêtla wpisuj¹ca wiersze z bazy do tabeli ImGui
                for (const auto& item : baza.lista_polaczen) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", item.id.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", item.start.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", item.stop.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // --- ZAK£ADKA: RUCH ---
        if (ImGui::BeginTabItem("Ruch")) {
            if (ImGui::BeginTable("RuchTable", 6, flags, ImVec2(0, 300))) {
                ImGui::TableSetupColumn("ID Ruchu");
                ImGui::TableSetupColumn("ID Polaczenia");
                ImGui::TableSetupColumn("ID Typu");
                ImGui::TableSetupColumn("Data Ruchu");
                ImGui::TableSetupColumn("Impuls x");
                ImGui::TableSetupColumn("Impuls y");
                ImGui::TableHeadersRow();

                for (const auto& item : baza.lista_ruchow) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", item.id.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", item.id_pol.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", item.id_typu.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", item.data.c_str());
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%s", item.x.c_str());
                    ImGui::TableSetColumnIndex(5); ImGui::Text("%s", item.y.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // --- ZAK£ADKA: STRZA£ ---
        if (ImGui::BeginTabItem("Strzal")) {
            if (ImGui::BeginTable("StrzalTable", 5, flags, ImVec2(0, 300))) {
                ImGui::TableSetupColumn("ID Strzalu");
                ImGui::TableSetupColumn("ID Polaczenia");
                ImGui::TableSetupColumn("ID Ruchu");
                ImGui::TableSetupColumn("ID Obiektu");
                ImGui::TableSetupColumn("Data strzalu");
                ImGui::TableHeadersRow();

                for (const auto& item : baza.lista_strzalow) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", item.id.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", item.id_pol.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", item.id_ruch.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", item.id_obj.c_str());
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%s", item.data.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // --- ZAK£ADKA: OBIEKT WYKRYTY ---
        if (ImGui::BeginTabItem("Obiekt wykryty")) {
            if (ImGui::BeginTable("ObiektTable", 4, flags, ImVec2(0, 300))) {
                ImGui::TableSetupColumn("Data wykrycia");
                ImGui::TableSetupColumn("Kategoria obiektu");
                ImGui::TableSetupColumn("szerokosc obiektu");
                ImGui::TableSetupColumn("wysokosc obiektu");
                ImGui::TableHeadersRow();

                for (const auto& item : baza.lista_obiektow) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", item.data.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", item.kategoria.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", item.szerokosc.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", item.wysokosc.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // --- ZAK£ADKA: STATYSTYKI ---
        if (ImGui::BeginTabItem("Statystki")) {
            ImGui::Text("Calkowita liczba zarejestrowanych ruchow: %d", (int)baza.lista_ruchow.size());
            ImGui::Text("Calkowita liczba oddanych strzalow: %d", (int)baza.lista_strzalow.size());
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
void RysujTrybyWiezy() {

    ImGui::SetNextWindowSize(ImVec2(150, 100), ImGuiCond_FirstUseEver);


    ImGui::Begin("Tryby Wiezyczki", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::End();
}

// Main code
int main(int, char**)
{
    InicjalizujPortCOM("\\\\.\\COM3");
    if (baza.Polacz()) {
        baza.Start();
    }
    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
   // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        RysujInterfejsWiezyczki();
        RysujBazeDanych();
        //RysujTrybyWiezy();
        RysujRadar();
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);


    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    baza.Rozlacz();
    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

