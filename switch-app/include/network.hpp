#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <vector>
#include <SDL.h>

struct SessionResponse {
    std::string sessionId;
    std::string authUrl;
    std::string status;
    std::string userName;
};

struct DriveFile {
    std::string id;
    std::string name;
    std::string coverUrl;
    std::string system;
    SDL_Texture* coverTexture = nullptr;
    bool isFolder;
};

class Network {
public:
    static SessionResponse initSession(const std::string& deviceToken);
    static SessionResponse checkStatus(const std::string& deviceToken);
    static SDL_Texture* downloadImage(const std::string& url);
    static std::vector<DriveFile> listDriveFiles(const std::string& userId, const std::string& folderId = "root");
    
    static bool uploadSave(uint64_t titleId, const std::string& filePath);
    static bool downloadSave(uint64_t titleId, const std::string& destinationPath);
};

#endif
