#include <switch.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "qrcodegen.hpp"

using json = nlohmann::json;
using namespace qrcodegen;

// Configuração do Portal (Altere para seu IP local ou domínio de produção)
const std::string BASE_URL = "http://192.168.0.15:3000";
const std::string APP_VERSION = "1.0.1";
const std::string APP_PATH = "/switch/AppSwitch.nro";

// Função callback para receber dados do HTTP
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Gera QR code em ASCII
void printQRCodeASCII(const std::string& text) {
    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);
    int size = qr.getSize();
    
    printf("\n");
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            printf("%s", qr.getModule(x, y) ? "██" : "  ");
        }
        printf("\n");
    }
    printf("\n");
}

// Gera deviceToken único do Switch
std::string getDeviceToken() {
    // Em produção, usar serial do console ou UUID único
    // Por enquanto, usa um valor fixo para teste
    return "switch-device-001";
}

// Cria sessão no backend
std::string createSession(const std::string& deviceToken) {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if(curl) {
        std::string url = BASE_URL + "/api/session/init";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        // JSON body
        std::string jsonBody = "{\"deviceToken\":\"" + deviceToken + "\"}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        
        // Headers
        struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            printf("Erro na requisição: %s\n", curl_easy_strerror(res));
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return response;
}

// Verifica versão do app
std::string checkAppVersion() {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if(curl) {
        std::string url = BASE_URL + "/api/app/version";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            printf("Erro ao verificar versão: %s\n", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

// Download do arquivo .nro
bool downloadUpdate(const std::string& url, const std::string& outputPath) {
    CURL* curl = curl_easy_init();
    FILE* fp = fopen(outputPath.c_str(), "wb");
    
    if(!curl || !fp) {
        if(fp) fclose(fp);
        if(curl) curl_easy_cleanup(curl);
        return false;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK);
}

// Verifica status da sessão
std::string checkSessionStatus(const std::string& deviceToken) {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if(curl) {
        std::string url = BASE_URL + "/api/session/status?deviceToken=" + deviceToken;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            printf("Erro na requisição: %s\n", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

// (Versão e caminhos movidos para o topo)

int main(int argc, char **argv) {
    consoleInit(NULL);
    
    // Inicializa os controles
    PadState pad;
    padInitializeDefault(&pad);

    // Inicializa curl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Verifica atualização ao iniciar
    printf("Portal NX-Cloud v%s\n", APP_VERSION.c_str());
    printf("--------------------\n");
    printf("Verificando atualizações...\n\n");
    
    std::string versionResponse = checkAppVersion();
    bool updateAvailable = false;
    std::string latestVersion;
    std::string downloadUrl;
    bool forceUpdate = false;
    
    try {
        auto versionJson = json::parse(versionResponse);
        if (versionJson.contains("version")) {
            latestVersion = versionJson["version"].get<std::string>();
            if (latestVersion != APP_VERSION) {
                updateAvailable = true;
                if (versionJson.contains("downloadUrl")) {
                    downloadUrl = versionJson["downloadUrl"].get<std::string>();
                }
                if (versionJson.contains("forceUpdate")) {
                    forceUpdate = versionJson["forceUpdate"].get<bool>();
                }
                
                printf("NOVA VERSÃO DISPONÍVEL!\n");
                printf("Versão atual: %s\n", APP_VERSION.c_str());
                printf("Nova versão: %s\n", latestVersion.c_str());
                
                if (versionJson.contains("changelog")) {
                    printf("\nChangelog: %s\n", versionJson["changelog"].get<std::string>().c_str());
                }
                
                printf("\nDeseja atualizar?\n");
                printf("(A) Sim - (B) Não\n\n");
            } else {
                printf("App está na versão mais recente!\n\n");
            }
        }
    } catch (...) {
        printf("Não foi possível verificar atualizações\n\n");
    }
    
    // Loop de atualização
    bool updating = updateAvailable;
    bool updateDone = false;
    
    while(appletMainLoop() && updating) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        
        if (kDown & HidNpadButton_A) {
            printf("Baixando atualização...\n");
            
            // Download para arquivo temporário
            std::string tempPath = APP_PATH + ".tmp";
            if (downloadUpdate(downloadUrl, tempPath)) {
                printf("Download concluído!\n");
                printf("Instalando atualização...\n");
                
                // Remove arquivo antigo e renomeia novo
                remove(APP_PATH.c_str());
                rename(tempPath.c_str(), APP_PATH.c_str());
                
                printf("Atualização instalada com sucesso!\n");
                printf("Reinicie o app para usar a nova versão.\n");
                updateDone = true;
                
                printf("\nPressione (+) para sair\n");
            } else {
                printf("Erro ao baixar atualização.\n");
                remove(tempPath.c_str());
            }
            
            if (!forceUpdate || updateDone) {
                updating = false;
            }
        }
        
        if (kDown & HidNpadButton_B) {
            if (!forceUpdate) {
                printf("Atualização ignorada.\n\n");
                updating = false;
            } else {
                printf("Esta atualização é obrigatória!\n");
            }
        }
        
        if (kDown & HidNpadButton_Plus && updateDone) {
            break;
        }
        
        consoleUpdate(NULL);
    }
    
    if (updateDone) {
        // Aguarda usuário sair após atualização
        while(appletMainLoop()) {
            padUpdate(&pad);
            if (padGetButtonsDown(&pad) & HidNpadButton_Plus) {
                break;
            }
            consoleUpdate(NULL);
        }
        
        curl_global_cleanup();
        consoleExit(NULL);
        return 0;
    }

    // Gera deviceToken
    std::string deviceToken = getDeviceToken();
    
    // Cria sessão
    std::string sessionResponse = createSession(deviceToken);
    
    // Parse JSON response
    std::string authUrl;
    try {
        auto jsonData = json::parse(sessionResponse);
        if (jsonData.contains("authUrl")) {
            authUrl = jsonData["authUrl"];
        }
    } catch (...) {
        authUrl = BASE_URL + "/auth?sessionId=error";
    }

    printf("--------------------\n");
    printf("Device Token: %s\n", deviceToken.c_str());
    printf("Auth URL: %s\n\n", authUrl.c_str());
    printf("QR Code:\n");
    printQRCodeASCII(authUrl);
    printf("Escaneie o QR code acima\n");
    printf("Pressione (A) para verificar status\n");
    printf("Pressione (+) para sair\n");

    bool connected = false;
    
    while(appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) {
            break;
        }

        if (kDown & HidNpadButton_A && !connected) {
            printf("Verificando status...\n");
            std::string status = checkSessionStatus(deviceToken);
            
            try {
                auto statusJson = json::parse(status);
                if (statusJson.contains("status") && statusJson["status"] == "CONNECTED") {
                    connected = true;
                    printf("\n=== CONECTADO! ===\n");
                    
                    if (statusJson.contains("user")) {
                        auto user = statusJson["user"];
                        if (user.contains("name")) {
                            printf("Usuário: %s\n", user["name"].get<std::string>().c_str());
                        } else if (user.contains("email")) {
                            printf("Email: %s\n", user["email"].get<std::string>().c_str());
                        }
                    }
                    
                    printf("\nPressione (+) para sair\n");
                } else {
                    printf("Status: PENDENTE\n");
                }
            } catch (...) {
                printf("Erro ao verificar status\n");
            }
        }

        consoleUpdate(NULL);
    }

    // Cleanup
    curl_global_cleanup();
    consoleExit(NULL);
    return 0;
}
