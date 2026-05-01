#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <SDL.h>
#include "graphics.hpp"
#include "discovery.hpp"
#include "network.hpp"

std::string formatBytes(s64 bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    u64 ubytes = (u64)bytes;
    if (ubytes >= 1024ULL * 1024ULL * 1024ULL) {
        ss << (ubytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    } else if (ubytes >= 1024ULL * 1024ULL) {
        ss << (ubytes / (1024.0 * 1024.0)) << " MB";
    } else {
        ss << (ubytes / 1024.0) << " KB";
    }
    return ss.str();
}

enum AppState {
    STATE_LOGIN,
    STATE_LIBRARY,
    STATE_DRIVE,
    STATE_GAME_DETAILS,
    STATE_ERROR
};

struct GlobalContext {
    AppState state = STATE_LOGIN;
    std::string userId; 
    std::string deviceToken;
    std::string authUrl;
    std::string userName;
    std::vector<GameInfo> games;
    std::vector<DriveFile> driveFiles;
    int selectedGame = 0;
    int selectedFile = 0;
} ctx;

PadState pad;

void handleInput() {
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);

    if (ctx.state == STATE_LIBRARY || ctx.state == STATE_DRIVE) {
        if (kDown & HidNpadButton_R) ctx.state = STATE_DRIVE;
        if (kDown & HidNpadButton_L) ctx.state = STATE_LIBRARY;
    }

    if (ctx.state == STATE_LIBRARY) {
        if (kDown & HidNpadButton_Right) ctx.selectedGame++;
        if (kDown & HidNpadButton_Left) ctx.selectedGame--;
        if (kDown & HidNpadButton_Down) ctx.selectedGame += 6;
        if (kDown & HidNpadButton_Up) ctx.selectedGame -= 6;
        if (ctx.selectedGame < 0) ctx.selectedGame = 0;
        if (ctx.selectedGame >= (int)ctx.games.size()) ctx.selectedGame = ctx.games.size() - 1;
    } 
    else if (ctx.state == STATE_DRIVE) {
        if (kDown & HidNpadButton_Down) ctx.selectedFile++;
        if (kDown & HidNpadButton_Up) ctx.selectedFile--;
        if (ctx.selectedFile < 0) ctx.selectedFile = 0;
        if (ctx.selectedFile >= (int)ctx.driveFiles.size()) ctx.selectedFile = ctx.driveFiles.size() - 1;
    }
}

void renderUI() {
    Graphics::clear();

    // Barra Superior
    Graphics::drawRect(0, 0, 1280, 60, {30, 30, 40, 255});
    Graphics::drawText("NX-Cloud (FREE VERSION)", 20, 15, 24, {0, 255, 150, 255});
    
    if (ctx.state == STATE_LIBRARY || ctx.state == STATE_DRIVE) {
        SDL_Color tabColor = (ctx.state == STATE_LIBRARY) ? SDL_Color{0, 255, 150, 255} : SDL_Color{150, 150, 150, 255};
        Graphics::drawText("L: BIBLIOTECA", 400, 15, 20, tabColor);
        tabColor = (ctx.state == STATE_DRIVE) ? SDL_Color{0, 255, 150, 255} : SDL_Color{150, 150, 150, 255};
        Graphics::drawText("R: GOOGLE DRIVE", 650, 15, 20, tabColor);
    }

    if (ctx.state == STATE_LOGIN) {
        Graphics::drawText("Conecte sua conta para sincronizar saves e instalar jogos", 380, 130, 22, {255, 255, 255, 255});
        if (!ctx.authUrl.empty()) {
            Graphics::drawQRCode(ctx.authUrl, 490, 180, 300);
            Graphics::drawText("Escaneie o QR Code no seu celular", 450, 500, 20, {200, 200, 200, 255});
        }
    } 
    else if (ctx.state == STATE_LIBRARY) {
        Graphics::drawText("Logado como: " + ctx.userName, 950, 15, 18, {255, 255, 255, 255});
        int startX = 100, startY = 100;
        for (int i = 0; i < (int)ctx.games.size(); i++) {
            int row = i / 6, col = i % 6;
            int x = startX + (col * 180), y = startY + (row * 180);
            if (i == ctx.selectedGame) Graphics::drawRect(x - 5, y - 5, 160, 160, {0, 255, 150, 255});
            if (ctx.games[i].icon) Graphics::drawImage(ctx.games[i].icon, x, y, 150, 150);
            else Graphics::drawRect(x, y, 150, 150, {50, 50, 60, 255});
        }
        if (!ctx.games.empty()) {
            Graphics::drawRect(0, 640, 1280, 80, {25, 25, 35, 255});
            Graphics::drawText("Jogo: " + ctx.games[ctx.selectedGame].name, 50, 665, 22, {255, 255, 255, 255});
            Graphics::drawText("(A) BACKUP    (Y) RESTAURAR", 850, 665, 22, {0, 255, 150, 255});
        }
    }
    else if (ctx.state == STATE_DRIVE) {
        Graphics::drawText("Navegador Google Drive:", 50, 80, 22, {255, 255, 255, 255});
        for (int i = 0; i < (int)ctx.driveFiles.size(); i++) {
            int y = 120 + (i * 40);
            if (i == ctx.selectedFile) Graphics::drawRect(45, y - 5, 1190, 35, {50, 50, 70, 255});
            
            std::string info = ctx.driveFiles[i].isFolder ? "📁 " : "🎮 ";
            info += ctx.driveFiles[i].name;
            if (!ctx.driveFiles[i].system.empty()) {
                info += " [" + ctx.driveFiles[i].system + "]";
            }
            
            Graphics::drawText(info, 60, y, 18, {220, 220, 220, 255});
        }
        Graphics::drawRect(0, 640, 1280, 80, {25, 25, 35, 255});
        Graphics::drawText("(A) INSTALAR JOGO NO SWITCH", 850, 665, 22, {0, 255, 150, 255});
    }

    Graphics::present();
}

int main(int argc, char* argv[]) {
    socketInitializeDefault();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    if (!Graphics::init()) return 0;

    bool nsReady = R_SUCCEEDED(nsInitialize());
    
    DeviceInfo deviceInfo;
    deviceInfo.deviceName = "Nintendo Switch";
    deviceInfo.sdTotal = "N/A";
    deviceInfo.sdFree = "N/A";
    deviceInfo.nandTotal = "N/A";
    deviceInfo.nandFree = "N/A";

    // Usa o serial do console como identificador persistente da sessão.
    if (R_SUCCEEDED(setsysInitialize())) {
        SetSysSerialNumber serial{};
        if (R_SUCCEEDED(setsysGetSerialNumber(&serial)) && serial.number[0] != '\0') {
            deviceInfo.deviceToken = serial.number;
            ctx.deviceToken = serial.number;
        } else {
            deviceInfo.deviceToken = "GENERIC_SWITCH_ID";
            ctx.deviceToken = "GENERIC_SWITCH_ID";
        }
        setsysExit();
    } else {
        deviceInfo.deviceToken = "GENERIC_SWITCH_ID";
        ctx.deviceToken = "GENERIC_SWITCH_ID";
    }

    // Pega informações de armazenamento
    FsFileSystem fs;
    if (R_SUCCEEDED(fsOpenBisFileSystem(&fs, FsBisPartitionId_User, ""))) {
        s64 total = 0, free = 0;
        fsFsGetTotalSpace(&fs, "/", &total);
        fsFsGetFreeSpace(&fs, "/", &free);
        deviceInfo.nandTotal = formatBytes(total);
        deviceInfo.nandFree = formatBytes(free);
        fsFsClose(&fs);
    }

    if (R_SUCCEEDED(fsOpenSdCardFileSystem(&fs))) {
        s64 total = 0, free = 0;
        fsFsGetTotalSpace(&fs, "/", &total);
        fsFsGetFreeSpace(&fs, "/", &free);
        deviceInfo.sdTotal = formatBytes(total);
        deviceInfo.sdFree = formatBytes(free);
        fsFsClose(&fs);
    }

    SessionResponse res = Network::initSession(deviceInfo);
    ctx.authUrl = res.authUrl;
    if (nsReady) {
        ctx.games = Discovery::listGames();
    }

    u64 lastPoll = 0;
    while (appletMainLoop()) {
        handleInput();
        if (ctx.state == STATE_LOGIN && (svcGetSystemTick() - lastPoll) > 2000000000ULL) {
            SessionResponse statusRes = Network::checkStatus(ctx.deviceToken);
            if (statusRes.status == "CONNECTED") {
                ctx.state = STATE_LIBRARY;
                ctx.userName = statusRes.userName;
            }
            lastPoll = svcGetSystemTick();
        }
        renderUI();
        if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
    }

    Discovery::cleanup(ctx.games);
    if (nsReady) nsExit();
    Graphics::exit();
    socketExit();
    return 0;
}
