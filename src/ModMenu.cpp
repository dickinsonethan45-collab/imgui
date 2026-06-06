#include "ModMenu.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>

bool  g_infiniteAmmo = false;
bool  g_godMode      = false;
bool  g_noClip       = false;
bool  g_speedHack    = false;
float g_speedMult    = 2.0f;
bool  g_esp          = false;
bool  g_espNames     = true;
bool  g_espBox       = false;
bool  g_chams        = false;
bool  g_friendlyFire = false;
float g_fov          = 90.0f;

static bool  s_open      = true;
static int   s_tab       = 0;
static char  s_roomCode[32] = "";
static std::vector<ACPlayer> s_players;
static int   s_selPlayer = -1;
static int   s_sbSel     = -1;
static const char* s_sounds[] = { "Fart","Airhorn","Vine Boom","Moan","Bruh","Rizz","Nyan Cat" };
static char  s_chatMsg[256]   = "";
static bool  s_spamChat       = false;
static float s_spamDelay      = 1.0f;
static bool  s_joinRandFF     = false;
static bool  s_autoRejoin     = false;
static float s_uiScale        = 3.0f;
static float s_opacity        = 0.93f;

static void PushStyle()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg,         ImVec4(0.05f,0.05f,0.08f,0.93f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,          ImVec4(0.07f,0.07f,0.12f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,    ImVec4(0.10f,0.10f,0.17f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_Button,           ImVec4(0.17f,0.17f,0.25f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,    ImVec4(0.27f,0.27f,0.38f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,     ImVec4(0.37f,0.37f,0.52f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.12f,0.12f,0.18f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0.20f,0.20f,0.28f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark,        ImVec4(0.20f,0.90f,0.40f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.20f,0.80f,0.40f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.30f,1.00f,0.50f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_Tab,              ImVec4(0.12f,0.12f,0.18f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered,       ImVec4(0.25f,0.25f,0.38f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_TabActive,        ImVec4(0.18f,0.52f,0.94f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_Header,           ImVec4(0.18f,0.18f,0.28f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,    ImVec4(0.25f,0.25f,0.40f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,     ImVec4(0.18f,0.52f,0.90f,1.00f));
    ImGui::PushStyleColor(ImGuiCol_Separator,        ImVec4(0.28f,0.28f,0.40f,0.60f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    ImVec2(6.0f,5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(10.0f,8.0f));
}

static void PopStyle()
{
    ImGui::PopStyleColor(18);
    ImGui::PopStyleVar(4);
}

static void ColText(ImVec4 col, const char* fmt, ...)
{
    char buf[512];
    va_list a; va_start(a,fmt);
    vsnprintf(buf,sizeof(buf),fmt,a);
    va_end(a);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextUnformatted(buf);
    ImGui::PopStyleColor();
}

#define GREEN ImVec4(0.20f,0.90f,0.40f,1.0f)
#define CYAN  ImVec4(0.40f,0.85f,1.00f,1.0f)

static void TabPlayer()
{
    if (ImGui::Button("Dock In Front of Face", ImVec2(-1,0))) {}
    ImGui::Spacing();
    ColText(GREEN,"Room Code:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::InputText("##rc", s_roomCode, sizeof(s_roomCode));
    ImGui::Spacing();
    ImGui::Checkbox("Friendly Fire", &g_friendlyFire);
}

static void TabExploits()
{
    ImGui::Checkbox("Infinite Ammo", &g_infiniteAmmo);
    ImGui::Checkbox("God Mode",      &g_godMode);
    ImGui::Checkbox("No Clip",       &g_noClip);
    ImGui::Separator();
    ImGui::Checkbox("Speed Hack",    &g_speedHack);
    if (g_speedHack) {
        ImGui::SetNextItemWidth(180);
        ImGui::SliderFloat("Mult##sp", &g_speedMult, 1.0f, 10.0f, "%.1fx");
    }
}

static void TabVisual()
{
    ImGui::Checkbox("ESP", &g_esp);
    if (g_esp) {
        ImGui::Indent();
        ImGui::Checkbox("Names", &g_espNames);
        ImGui::Checkbox("Boxes", &g_espBox);
        ImGui::Unindent();
    }
    ImGui::Separator();
    ImGui::Checkbox("Chams", &g_chams);
    ImGui::Separator();
    ImGui::SetNextItemWidth(180);
    ImGui::SliderFloat("FOV##v", &g_fov, 60.0f, 140.0f, "%.0f");
}

static void TabPlayers()
{
    ImGui::BeginChild("##plist", ImVec2(-1,160), true);
    for (int i = 0; i < (int)s_players.size(); i++) {
        auto& p = s_players[i];
        char label[128];
        snprintf(label,sizeof(label),"  %s%s",p.name.c_str(),p.isMeta?" [META]":"");
        ImGui::PushID(i);
        if (ImGui::Selectable(label, s_selPlayer==i, 0, ImVec2(-1,18)))
            s_selPlayer = i;
        ImGui::PopID();
    }
    ImGui::EndChild();

    if (s_selPlayer >= 0 && s_selPlayer < (int)s_players.size()) {
        auto& p = s_players[s_selPlayer];
        ImGui::Separator();
        if (!p.userId.empty())     ColText(CYAN,"ID: %s",          p.userId.c_str());
        if (!p.facebookId.empty()) ColText(CYAN,"Facebook ID: %s", p.facebookId.c_str());
        ColText(CYAN,"Username: %s | Friends: %d", p.name.c_str(), p.friendCount);
        ImGui::Spacing();

        if (ImGui::Button("Teleport To")) {} ImGui::SameLine();
        if (ImGui::Button("TP To Me"))    {} ImGui::SameLine();
        if (ImGui::Button("TP Void"))     {}

        if (ImGui::Button("Shake Screen")){ } ImGui::SameLine();
        if (ImGui::Button("Color"))        { } ImGui::SameLine();
        if (ImGui::Button("Fling Up"))     { }

        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f,0.10f,0.10f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f,0.15f,0.15f,1.0f));
        if (ImGui::Button("Kick"))  {}
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        if (ImGui::Button("Save Info")) {}
    }
}

static void TabSoundboard()
{
    ImGui::TextDisabled("Tap to play:");
    ImGui::Spacing();
    for (int i = 0; i < IM_ARRAYSIZE(s_sounds); i++) {
        bool sel = (s_sbSel==i);
        if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f,0.52f,0.90f,1.0f));
        ImGui::PushID(i);
        if (ImGui::Button(s_sounds[i], ImVec2(120,0))) s_sbSel=i;
        ImGui::PopID();
        if (sel) ImGui::PopStyleColor();
        if ((i%3)!=2) ImGui::SameLine();
    }
}

static void TabText()
{
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##cm", s_chatMsg, sizeof(s_chatMsg));
    ImGui::Spacing();
    if (ImGui::Button("Send Chat",ImVec2(120,0))) {}
    ImGui::SameLine();
    if (ImGui::Button("Clear",ImVec2(70,0))) s_chatMsg[0]='\0';
    ImGui::Separator();
    ImGui::Checkbox("Spam Chat", &s_spamChat);
    if (s_spamChat) {
        ImGui::SetNextItemWidth(160);
        ImGui::SliderFloat("Delay##sp", &s_spamDelay, 0.1f, 5.0f, "%.1fs");
    }
}

static void TabMisc()
{
    ImGui::Checkbox("Join Random (No FF)", &s_joinRandFF);
    ImGui::Checkbox("Auto Re-join",        &s_autoRejoin);
}

static void TabCredits()
{
    ImGui::Spacing();
    ColText(GREEN,"  Animal Company Mod Menu");
    ImGui::TextDisabled("  EGL hook + Dear ImGui");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("  Dev:"); ImGui::SameLine(); ColText(CYAN,"Cheese");
    ImGui::Spacing();
    ImGui::TextDisabled("  v1.0.0");
}

static void TabConfig()
{
    ImGui::TextDisabled("Menu settings");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::SetNextItemWidth(180);
    if (ImGui::SliderFloat("UI Scale",&s_uiScale,0.5f,5.0f,"%.2f"))
        ImGui::GetIO().FontGlobalScale = s_uiScale;
    ImGui::SetNextItemWidth(180);
    if (ImGui::SliderFloat("Opacity",&s_opacity,0.2f,1.0f,"%.2f"))
        ImGui::GetStyle().Alpha = s_opacity;
    ImGui::Spacing();
    if (ImGui::Button("Reset",ImVec2(-1,0))) {
        s_uiScale=3.0f; s_opacity=0.93f;
        ImGui::GetIO().FontGlobalScale=s_uiScale;
        ImGui::GetStyle().Alpha=s_opacity;
    }
}

void Menu_SetPlayers(const std::vector<ACPlayer>& players)
{
    s_players=players;
    s_selPlayer=-1;
}

void DrawMenu()
{
    if (!s_open) return;
    PushStyle();

    ImGui::SetNextWindowSize(ImVec2(520,400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50,50),    ImGuiCond_FirstUseEver);

    ImGui::Begin("Moony's Animal Company menu <3", &s_open,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    if (ImGui::BeginTabBar("##tabs")) {
        const char* names[] = {
            "Player","Exploits","Visual","Players",
            "Soundboard","Text","Misc","Credits","Config"
        };
        for (int i = 0; i < IM_ARRAYSIZE(names); i++)
            if (ImGui::BeginTabItem(names[i])) { s_tab=i; ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::Spacing();
    switch(s_tab) {
        case 0: TabPlayer();     break;
        case 1: TabExploits();   break;
        case 2: TabVisual();     break;
        case 3: TabPlayers();    break;
        case 4: TabSoundboard(); break;
        case 5: TabText();       break;
        case 6: TabMisc();       break;
        case 7: TabCredits();    break;
        case 8: TabConfig();     break;
    }

    ImGui::End();
    PopStyle();
}
