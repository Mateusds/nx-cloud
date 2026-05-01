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

const std::string APP_VERSION = "v14";

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
    STATE_UPDATE,
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
    std::string sdInfo;
    std::string nandInfo;
    u64 statusTimeout = 0;
    bool updating = false;
} ctx;

PadState pad;

// ═══════════════════════════════════════════════════════════════════
//  INPUT HANDLER
// ═══════════════════════════════════════════════════════════════════

void handleInputCustom(u64 kDown) {
    // (+) sempre vai para tela de atualizacoes
    if (kDown & HidNpadButton_Plus) {
        if (ctx.state != STATE_UPDATE) {
            ctx.state = STATE_UPDATE;
            ctx.newVersion = Network::checkUpdate();
            return;
        }
    }

    // Navegacao por abas (L/R)
    if (ctx.state == STATE_LIBRARY || ctx.state == STATE_DRIVE) {
        if (kDown & HidNpadButton_R) {
            ctx.state = STATE_DRIVE;
            if (ctx.driveFiles.empty() && !ctx.userId.empty()) {
                ctx.statusMessage = "Carregando Google Drive...";
                ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
                ctx.driveFiles = Network::listDriveFiles(ctx.userId);
                if (ctx.driveFiles.empty()) {
                    ctx.statusMessage = "Nenhum arquivo encontrado.";
                } else {
                    ctx.statusMessage = std::to_string(ctx.driveFiles.size()) + " arquivo(s) carregado(s)!";
                }
                ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
            } else if (ctx.userId.empty()) {
                ctx.statusMessage = "Drive requer login via QR Code.";
                ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
            }
        }
        if (kDown & HidNpadButton_L) ctx.state = STATE_LIBRARY;
    }

    // Selecao na Biblioteca
    if (ctx.state == STATE_LIBRARY && !ctx.games.empty()) {
        if (kDown & HidNpadButton_Right) ctx.selectedGame++;
        if (kDown & HidNpadButton_Left) ctx.selectedGame--;
        if (kDown & HidNpadButton_Down) ctx.selectedGame += 5;
        if (kDown & HidNpadButton_Up) ctx.selectedGame -= 5;
        if (ctx.selectedGame < 0) ctx.selectedGame = 0;
        if (ctx.selectedGame >= (int)ctx.games.size()) ctx.selectedGame = (int)ctx.games.size() - 1;

        if (kDown & HidNpadButton_A) {
            ctx.statusMessage = "BACKUP: " + ctx.games[ctx.selectedGame].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        }
        if (kDown & HidNpadButton_Y) {
            ctx.statusMessage = "RESTAURAR: " + ctx.games[ctx.selectedGame].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        }
    }

    // Selecao no Drive
    if (ctx.state == STATE_DRIVE && !ctx.driveFiles.empty()) {
        if (kDown & HidNpadButton_Down) ctx.selectedFile++;
        if (kDown & HidNpadButton_Up) ctx.selectedFile--;
        if (ctx.selectedFile < 0) ctx.selectedFile = 0;
        if (ctx.selectedFile >= (int)ctx.driveFiles.size()) ctx.selectedFile = (int)ctx.driveFiles.size() - 1;

        if (kDown & HidNpadButton_A) {
            ctx.statusMessage = "INSTALANDO: " + ctx.driveFiles[ctx.selectedFile].name;
            ctx.statusTimeout = svcGetSystemTick() + 57600000ULL;
        }
    }

    // B volta para biblioteca ou sai do app
    if (kDown & HidNpadButton_B) {
        if (ctx.state == STATE_DRIVE) ctx.state = STATE_LIBRARY;
        else if (ctx.state == STATE_UPDATE) ctx.state = STATE_LIBRARY;
    }

    // Tela de Update
    if (ctx.state == STATE_UPDATE && !ctx.updating) {
        if (kDown & HidNpadButton_A && !ctx.newVersion.empty() && ctx.newVersion != APP_VERSION) {
            ctx.updating = true;
            ctx.statusMessage = "Baixando " + ctx.newVersion + "...";
            ctx.statusTimeout = svcGetSystemTick() + 2000000000ULL;

            std::string updateUrl = "https://github.com/Mateusds/nx-cloud/releases/latest/download/AppSwitch.nro";
            if (Network::downloadFile(updateUrl, ctx.nroPath)) {
                ctx.statusMessage = "Atualizado! Reinicie o app.";
            } else {
                ctx.statusMessage = "Falha no download.";
            }
            ctx.statusTimeout = svcGetSystemTick() + 150000000ULL;
            ctx.updating = false;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  RENDER - NOVO LAYOUT
// ═══════════════════════════════════════════════════════════════════

void renderUI() {
    Graphics::clear();

    // ── HEADER ──────────────────────────────────────────────────
    Graphics::drawRect(0, 0, 1280, 50, {18, 18, 28, 255});
    Graphics::drawRect(0, 50, 1280, 2, {0, 200, 120, 255}); // linha verde
    Graphics::drawText("NX-CLOUD", 30, 14, 22, {0, 230, 140, 255});

    if (ctx.state != STATE_LOGIN && ctx.state != STATE_UPDATE) {
        // Abas
        SDL_Color libC = (ctx.state == STATE_LIBRARY) ? SDL_Color{0, 230, 140, 255} : SDL_Color{100, 100, 120, 255};
        SDL_Color drvC = (ctx.state == STATE_DRIVE) ? SDL_Color{0, 230, 140, 255} : SDL_Color{100, 100, 120, 255};
        Graphics::drawText("[L] JOGOS", 350, 16, 18, libC);
        Graphics::drawText("[R] DRIVE", 550, 16, 18, drvC);
        
        // Nome do usuario
        if (!ctx.userName.empty()) {
            Graphics::drawText(ctx.userName, 1050, 16, 16, {180, 180, 200, 255});
        }
    }

    // ── CONTENT AREA ────────────────────────────────────────────

    if (ctx.state == STATE_LOGIN) {
        // Sidebar info
        Graphics::drawRect(0, 52, 400, 588, {22, 22, 35, 255});
        Graphics::drawText("Bem-vindo ao", 50, 120, 22, {150, 150, 170, 255});
        Graphics::drawText("NX-Cloud", 50, 155, 36, {0, 230, 140, 255});
        Graphics::drawText("Gerencie saves e jogos", 50, 210, 16, {130, 130, 150, 255});
        Graphics::drawText("direto do seu Switch.", 50, 235, 16, {130, 130, 150, 255});

        Graphics::drawText("COMO FUNCIONA:", 50, 310, 16, {0, 200, 120, 255});
        Graphics::drawText("1. Escaneie o QR Code", 50, 340, 14, {170, 170, 190, 255});
        Graphics::drawText("2. Faca login com Google", 50, 365, 14, {170, 170, 190, 255});
        Graphics::drawText("3. Autorize o Google Drive", 50, 390, 14, {170, 170, 190, 255});
        Graphics::drawText("4. Pronto! Volte ao Switch", 50, 415, 14, {170, 170, 190, 255});

        Graphics::drawText(APP_VERSION, 50, 590, 14, {80, 80, 100, 255});

        // QR Code area
        if (!ctx.authUrl.empty()) {
            Graphics::drawRect(500, 120, 350, 350, {255, 255, 255, 255});
            Graphics::drawQRCode(ctx.authUrl, 515, 135, 320);
            Graphics::drawText("Escaneie com seu celular", 520, 500, 18, {200, 200, 220, 255});
            Graphics::drawText("para conectar sua conta", 530, 530, 16, {140, 140, 160, 255});
        } else {
            Graphics::drawText("Conectando ao servidor...", 550, 300, 20, {200, 200, 200, 255});
        }
    }
    else if (ctx.state == STATE_LIBRARY) {
        // Sidebar esquerda com info
        Graphics::drawRect(0, 52, 240, 588, {22, 22, 35, 255});
        Graphics::drawText("BIBLIOTECA", 20, 70, 18, {0, 200, 120, 255});
        Graphics::drawText(std::to_string(ctx.games.size()) + " jogos", 20, 100, 14, {130, 130, 150, 255});

        if (!ctx.nandInfo.empty()) {
            Graphics::drawText("NAND", 20, 150, 12, {100, 100, 120, 255});
            Graphics::drawText(ctx.nandInfo, 20, 170, 14, {180, 180, 200, 255});
        }
        if (!ctx.sdInfo.empty()) {
            Graphics::drawText("SD CARD", 20, 210, 12, {100, 100, 120, 255});
            Graphics::drawText(ctx.sdInfo, 20, 230, 14, {180, 180, 200, 255});
        }

        // Jogo selecionado info
        if (!ctx.games.empty()) {
            Graphics::drawRect(10, 300, 220, 2, {40, 40, 60, 255});
            Graphics::drawText("SELECIONADO:", 20, 320, 12, {100, 100, 120, 255});
            // Truncar nome se muito longo
            std::string gameName = ctx.games[ctx.selectedGame].name;
            if (gameName.length() > 20) gameName = gameName.substr(0, 18) + "..";
            Graphics::drawText(gameName, 20, 345, 14, {255, 255, 255, 255});

            Graphics::drawText("(A) Backup Save", 20, 400, 14, {0, 200, 120, 255});
            Graphics::drawText("(Y) Restaurar Save", 20, 425, 14, {255, 200, 50, 255});
        }

        // Grid de jogos
        int startX = 270, startY = 72;
        int cardW = 140, cardH = 140, gap = 15;
        int cols = 5;
        for (int i = 0; i < (int)ctx.games.size(); i++) {
            int row = i / cols, col = i % cols;
            int x = startX + col * (cardW + gap);
            int y = startY + row * (cardH + gap + 20);

            if (i == ctx.selectedGame) {
                Graphics::drawRect(x - 4, y - 4, cardW + 8, cardH + 8, {0, 230, 140, 255});
            }
            if (ctx.games[i].icon) {
                Graphics::drawImage(ctx.games[i].icon, x, y, cardW, cardH);
            } else {
                Graphics::drawRect(x, y, cardW, cardH, {40, 40, 55, 255});
                // Primeira letra do jogo
                std::string initial = ctx.games[i].name.substr(0, 1);
                Graphics::drawText(initial, x + 55, y + 50, 30, {80, 80, 100, 255});
            }
        }
    }
    else if (ctx.state == STATE_DRIVE) {
        // Sidebar
        Graphics::drawRect(0, 52, 240, 588, {22, 22, 35, 255});
        Graphics::drawText("GOOGLE DRIVE", 20, 70, 18, {0, 200, 120, 255});
        Graphics::drawText(std::to_string(ctx.driveFiles.size()) + " itens", 20, 100, 14, {130, 130, 150, 255});

        if (!ctx.driveFiles.empty()) {
            Graphics::drawRect(10, 300, 220, 2, {40, 40, 60, 255});
            Graphics::drawText("SELECIONADO:", 20, 320, 12, {100, 100, 120, 255});
            std::string fn = ctx.driveFiles[ctx.selectedFile].name;
            if (fn.length() > 20) fn = fn.substr(0, 18) + "..";
            Graphics::drawText(fn, 20, 345, 14, {255, 255, 255, 255});
            Graphics::drawText("(A) Instalar", 20, 400, 14, {0, 200, 120, 255});
        }

        // Lista de arquivos
        int listY = 72;
        for (int i = 0; i < (int)ctx.driveFiles.size() && i < 14; i++) {
            int y = listY + i * 38;
            if (i == ctx.selectedFile) {
                Graphics::drawRect(245, y - 3, 1030, 34, {35, 35, 55, 255});
                Graphics::drawRect(245, y - 3, 4, 34, {0, 200, 120, 255}); // indicador
            }

            std::string prefix = ctx.driveFiles[i].isFolder ? "[DIR] " : "      ";
            std::string entry = prefix + ctx.driveFiles[i].name;
            if (!ctx.driveFiles[i].system.empty()) {
                entry += "  [" + ctx.driveFiles[i].system + "]";
            }
            SDL_Color fc = (i == ctx.selectedFile) ? SDL_Color{255, 255, 255, 255} : SDL_Color{170, 170, 190, 255};
            Graphics::drawText(entry, 260, y + 5, 16, fc);
        }

        if (ctx.driveFiles.empty()) {
            Graphics::drawText("Nenhum arquivo disponivel.", 500, 300, 20, {120, 120, 140, 255});
            Graphics::drawText("Conecte o Google Drive no portal web.", 430, 340, 16, {100, 100, 120, 255});
        }
    }
    else if (ctx.state == STATE_UPDATE) {
        // Tela de atualizacoes centralizada
        Graphics::drawRect(200, 100, 880, 460, {25, 25, 40, 255});
        Graphics::drawRect(200, 100, 880, 4, {0, 200, 120, 255}); // linha topo

        Graphics::drawText("ATUALIZACOES", 450, 130, 28, {0, 230, 140, 255});

        Graphics::drawText("Versao Instalada:", 300, 210, 18, {130, 130, 150, 255});
        Graphics::drawText(APP_VERSION, 600, 210, 18, {255, 255, 255, 255});

        Graphics::drawText("Ultima Release:", 300, 260, 18, {130, 130, 150, 255});
        if (!ctx.newVersion.empty()) {
            SDL_Color vc = (ctx.newVersion != APP_VERSION) ? SDL_Color{255, 200, 50, 255} : SDL_Color{0, 230, 140, 255};
            Graphics::drawText(ctx.newVersion, 600, 260, 18, vc);
        } else {
            Graphics::drawText("Verificando...", 600, 260, 18, {150, 150, 170, 255});
        }

        Graphics::drawText("Fonte:", 300, 310, 18, {130, 130, 150, 255});
        Graphics::drawText("GitHub (Mateusds/nx-cloud)", 600, 310, 18, {180, 180, 200, 255});

        if (!ctx.newVersion.empty() && ctx.newVersion != APP_VERSION) {
            Graphics::drawRect(350, 400, 250, 50, {0, 180, 100, 255});
            Graphics::drawText("(A) ATUALIZAR", 395, 415, 20, {255, 255, 255, 255});
        } else if (!ctx.newVersion.empty()) {
            Graphics::drawText("Voce esta na versao mais recente!", 390, 410, 18, {0, 200, 120, 255});
        }

        Graphics::drawRect(680, 400, 250, 50, {60, 60, 80, 255});
        Graphics::drawText("(B) VOLTAR", 740, 415, 20, {200, 200, 220, 255});
    }

    // ── TOAST ───────────────────────────────────────────────────
    if (!ctx.statusMessage.empty() && svcGetSystemTick() < ctx.statusTimeout) {
        Graphics::drawRect(380, 600, 520, 36, {30, 30, 100, 255});
        Graphics::drawRect(380, 600, 4, 36, {0, 200, 120, 255});
        Graphics::drawText(ctx.statusMessage, 400, 608, 16, {255, 255, 255, 255});
    } else if (svcGetSystemTick() >= ctx.statusTimeout) {
        ctx.statusMessage = "";
    }

    // ── FOOTER ──────────────────────────────────────────────────
    Graphics::drawRect(0, 668, 1280, 52, {14, 14, 22, 255});
    Graphics::drawRect(0, 668, 1280, 1, {40, 40, 60, 255}); // separador

    if (ctx.state == STATE_LOGIN) {
        Graphics::drawText("Aguardando autenticacao...", 30, 685, 14, {100, 100, 120, 255});
    } else {
        Graphics::drawText("[L/R] Aba", 30, 685, 14, {100, 100, 120, 255});
        Graphics::drawText("[DPAD] Navegar", 200, 685, 14, {100, 100, 120, 255});
        Graphics::drawText("[A] Selecionar", 420, 685, 14, {100, 100, 120, 255});
        Graphics::drawText("[B] Voltar", 620, 685, 14, {100, 100, 120, 255});
        Graphics::drawText("[+] Atualizacoes", 780, 685, 14, {100, 100, 120, 255});
    }

    // Versao no canto direito do footer
    Graphics::drawText(APP_VERSION, 1180, 685, 12, {60, 60, 80, 255});

    Graphics::present();
}

// ═══════════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════════

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

    // Serial do console como identificador
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

    // Armazenamento
    FsFileSystem fs;
    if (R_SUCCEEDED(fsOpenBisFileSystem(&fs, FsBisPartitionId_User, ""))) {
        s64 total = 0, free = 0;
        fsFsGetTotalSpace(&fs, "/", &total);
        fsFsGetFreeSpace(&fs, "/", &free);
        deviceInfo.nandTotal = formatBytes(total);
        deviceInfo.nandFree = formatBytes(free);
        ctx.nandInfo = deviceInfo.nandFree + " / " + deviceInfo.nandTotal;
        fsFsClose(&fs);
    }

    if (R_SUCCEEDED(fsOpenSdCardFileSystem(&fs))) {
        s64 total = 0, free = 0;
        fsFsGetTotalSpace(&fs, "/", &total);
        fsFsGetFreeSpace(&fs, "/", &free);
        deviceInfo.sdTotal = formatBytes(total);
        deviceInfo.sdFree = formatBytes(free);
        ctx.sdInfo = deviceInfo.sdFree + " / " + deviceInfo.sdTotal;
        fsFsClose(&fs);
    }

    if (nsReady) {
        ctx.games = Discovery::listGames();
    }

    // Verifica atualizacoes
    ctx.newVersion = Network::checkUpdate();

    // Sempre atualiza os dados de storage no servidor ao iniciar
    SessionResponse res = Network::initSession(deviceInfo);
    ctx.authUrl = res.authUrl;

    // Reconecta automaticamente
    SessionResponse statusRes = Network::checkStatus(ctx.deviceToken);
    if (statusRes.status == "CONNECTED") {
        ctx.state = STATE_LIBRARY;
        ctx.userName = statusRes.userName;
        ctx.userId = statusRes.userId;
    }

    u64 lastPoll = 0;
    bool running = true;
    while (appletMainLoop() && running) {
        // 1. Atualiza controles uma única vez por frame
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        // 2. Atalhos globais rápidos
        if (kDown & HidNpadButton_Plus) {
            if (ctx.state != STATE_UPDATE) {
                ctx.state = STATE_UPDATE;
                ctx.newVersion = Network::checkUpdate();
            }
        }
        if (kDown & HidNpadButton_Minus) running = false;

        // 3. Processa entrada do estado atual (sem padUpdate interno)
        handleInputCustom(kDown);

        // 4. Polling de login (apenas se estiver em repouso na tela de login)
        u64 currentTick = svcGetSystemTick();
        if (ctx.state == STATE_LOGIN && (currentTick - lastPoll) > 38400000ULL) { // ~2 segundos
            SessionResponse sr = Network::checkStatus(ctx.deviceToken);
            if (sr.status == "CONNECTED") {
                ctx.state = STATE_LIBRARY;
                ctx.userName = sr.userName;
                ctx.userId = sr.userId;
            }
            lastPoll = currentTick;
        }

        // 5. Desenha a tela
        renderUI();
    }

    Discovery::cleanup(ctx.games);
    if (nsReady) nsExit();
    Graphics::exit();
    socketExit();
    return 0;
}
