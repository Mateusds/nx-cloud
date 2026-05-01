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

const std::string APP_VERSION = "v7";
const std::string UPDATE_SOURCE = "GitHub (Mateusds/nx-cloud)";

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
    STATE_UPDATE_PROMPT,
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
    std::string newVersion;
    std::string statusMessage;
    std::string nroPath = "/switch/AppSwitch.nro";
    u64 statusTimeout = 0;
} ctx;

PadState pad;

void handleInput() {
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);

    if (ctx.state == STATE_LIBRARY || ctx.state == STATE_DRIVE) {
        if (kDown & HidNpadButton_R) {
            ctx.state = STATE_DRIVE;
            if (ctx.userId.empty()) {
                ctx.statusMessage = "Google Drive requer login no portal web.";
                ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
            } else if (ctx.driveFiles.empty()) {
                ctx.statusMessage = "Carregando Google Drive...";
                ctx.driveFiles = Network::listDriveFiles(ctx.userId);
                if (ctx.driveFiles.empty()) {
                    ctx.statusMessage = "Nenhum arquivo encontrado.";
                } else {
                    ctx.statusMessage = "Arquivos carregados!";
                }
                ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
            }
        }
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

    // Ações de Botões (A, Y, B)
    if (kDown & HidNpadButton_A) {
        if (ctx.state == STATE_LIBRARY && !ctx.games.empty()) {
            ctx.statusMessage = "BACKUP: " + ctx.games[ctx.selectedGame].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        } else if (ctx.state == STATE_DRIVE && !ctx.driveFiles.empty()) {
            ctx.statusMessage = "INSTALANDO: " + ctx.driveFiles[ctx.selectedFile].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        }
    }

    if (kDown & HidNpadButton_Y) {
        if (ctx.state == STATE_LIBRARY && !ctx.games.empty()) {
            ctx.statusMessage = "RESTAURANDO: " + ctx.games[ctx.selectedGame].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        }
    }

    if (ctx.state == STATE_DRIVE) {
        // Volta para a biblioteca se estiver no drive
        if (ctx.state == STATE_DRIVE) ctx.state = STATE_LIBRARY;
    }

    if (ctx.state == STATE_UPDATE_PROMPT) {
        if (kDown & HidNpadButton_A) {
            ctx.statusMessage = "Baixando v" + ctx.newVersion + "...";
            ctx.statusTimeout = svcGetSystemTick() + 2000000000ULL; // Longo tempo
            
            std::string updateUrl = "https://github.com/Mateusds/nx-cloud/releases/latest/download/AppSwitch.nro";
            if (Network::downloadFile(updateUrl, ctx.nroPath)) {
                ctx.statusMessage = "Sucesso! Reinicie o app para aplicar.";
            } else {
                ctx.statusMessage = "Erro no download. Tente novamente.";
            }
            ctx.statusTimeout = svcGetSystemTick() + 100000000ULL; 
        }
        if (kDown & HidNpadButton_B) {
            ctx.state = STATE_LIBRARY;
        }
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
    }
    else if (ctx.state == STATE_UPDATE_PROMPT) {
        Graphics::drawRect(200, 150, 880, 400, {40, 40, 60, 255});
        Graphics::drawText("NOVA ATUALIZACAO DISPONIVEL", 350, 200, 30, {0, 255, 150, 255});
        Graphics::drawText("Versao Atual: " + APP_VERSION, 350, 260, 20, {200, 200, 200, 255});
        Graphics::drawText("Nova Versao:  " + ctx.newVersion, 350, 300, 20, {255, 255, 255, 255});
        
        Graphics::drawText("Deseja atualizar agora?", 350, 380, 22, {255, 255, 255, 255});
        Graphics::drawText("(A) SIM, ATUALIZAR AGORA", 350, 440, 22, {0, 255, 150, 255});
        Graphics::drawText("(B) NAO, DEPOIS", 350, 480, 22, {255, 100, 100, 255});
    }

    // Notificação de Atualização (discreta se não estiver no prompt)
    if (ctx.state != STATE_UPDATE_PROMPT && !ctx.newVersion.empty() && ctx.newVersion != APP_VERSION) {
        Graphics::drawRect(340, 20, 600, 40, {255, 50, 50, 255});
        Graphics::drawText("NOVA ATUALIZACAO DISPONIVEL: " + ctx.newVersion, 360, 30, 20, {255, 255, 255, 255});
    }

    // Mensagens de Status (Toast)
    if (!ctx.statusMessage.empty() && svcGetSystemTick() < ctx.statusTimeout) {
        Graphics::drawRect(400, 580, 480, 40, {50, 50, 150, 255});
        Graphics::drawText(ctx.statusMessage, 420, 590, 16, {255, 255, 255, 255});
    } else if (svcGetSystemTick() >= ctx.statusTimeout) {
        ctx.statusMessage = "";
    }

    // Rodapé com guia de botões
    Graphics::drawRect(0, 640, 1280, 80, {25, 25, 35, 255});
    if (ctx.state == STATE_LIBRARY && !ctx.games.empty()) {
        Graphics::drawText("Jogo: " + ctx.games[ctx.selectedGame].name, 50, 655, 20, {255, 255, 255, 255});
        Graphics::drawText("(A) BACKUP    (Y) RESTAURAR", 850, 655, 20, {0, 255, 150, 255});
    } else if (ctx.state == STATE_DRIVE) {
        Graphics::drawText("(A) INSTALAR JOGO NO SWITCH", 850, 655, 20, {0, 255, 150, 255});
    }

    // Atalhos globais
    Graphics::drawText("(+) SAIR   (L/R) ALTERAR ABA   (DPAD) NAVEGAR", 50, 690, 16, {150, 150, 150, 255});

    // Informações de versão
    Graphics::drawText("Versão Atual: " + APP_VERSION, 1050, 675, 14, {120, 120, 120, 255});
    if (!ctx.newVersion.empty()) {
        Graphics::drawText("Última Release: " + ctx.newVersion, 1050, 692, 14, {150, 150, 150, 255});
    }

    Graphics::present();
}

int main(int argc, char* argv[]) {
    if (argc > 0) ctx.nroPath = argv[0];
    
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

    if (nsReady) {
        ctx.games = Discovery::listGames();
    }

    // Verifica atualizações no GitHub
    ctx.newVersion = Network::checkUpdate();
    if (!ctx.newVersion.empty() && ctx.newVersion != APP_VERSION) {
        ctx.state = STATE_UPDATE_PROMPT;
    }

    // Tenta reconectar automaticamente se já estiver vinculado
    SessionResponse statusRes = Network::checkStatus(ctx.deviceToken);
    if (statusRes.status == "CONNECTED") {
        ctx.state = STATE_LIBRARY;
        ctx.userName = statusRes.userName;
        ctx.userId = statusRes.userId;
    } else {
        SessionResponse res = Network::initSession(deviceInfo);
        ctx.authUrl = res.authUrl;
    }

    u64 lastPoll = 0;
    while (appletMainLoop()) {
        handleInput();
        if (ctx.state == STATE_LOGIN && (svcGetSystemTick() - lastPoll) > 38400000ULL) { // 2 segundos (19.2MHz * 2)
            SessionResponse statusRes = Network::checkStatus(ctx.deviceToken);
            if (statusRes.status == "CONNECTED") {
                ctx.state = STATE_LIBRARY;
                ctx.userName = statusRes.userName;
                ctx.userId = statusRes.userId;
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
