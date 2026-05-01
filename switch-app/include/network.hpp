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
    std::string userId;
};

struct DriveFile {
    std::string id;
    std::string name;
    std::string coverUrl;
    std::string system;
    SDL_Texture* coverTexture = nullptr;
    bool isFolder;
};

struct DeviceInfo {
    std::string deviceToken;
    std::string deviceName;
    std::string sdTotal;
    std::string sdFree;
    std::string nandTotal;
    std::string nandFree;
};

class Network {
public:
    static SessionResponse initSession(const DeviceInfo& info);
    static SessionResponse checkStatus(const std::string& deviceToken);
    static SDL_Texture* downloadImage(const std::string& url);
    static std::vector<DriveFile> listDriveFiles(const std::string& userId, const std::string& folderId = "root");
    
    static bool uploadSave(uint64_t titleId, const std::string& filePath);
    static bool downloadSave(uint64_t titleId, const std::string& destinationPath);
    static std::string checkUpdate();
    static bool downloadFile(const std::string& url, const std::string& path);
};

#endif
