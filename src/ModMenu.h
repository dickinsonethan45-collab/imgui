#pragma once
#include <string>
#include <vector>

struct ACPlayer {
    std::string name;
    std::string userId;
    std::string facebookId;
    int         friendCount;
    bool        isMeta;
    int         actorId;
};

void DrawMenu();
void Menu_SetPlayers(const std::vector<ACPlayer>& players);

extern bool  g_infiniteAmmo;
extern bool  g_godMode;
extern bool  g_noClip;
extern bool  g_speedHack;
extern float g_speedMult;
extern bool  g_esp;
extern bool  g_espNames;
extern bool  g_espBox;
extern bool  g_chams;
extern bool  g_friendlyFire;
extern float g_fov;
