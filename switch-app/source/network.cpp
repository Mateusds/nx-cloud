#include "network.hpp"
#include "graphics.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

const std::string BASE_URL = "https://nx-cloud.mateusmarquesds.com";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

SessionResponse Network::initSession(const std::string& deviceToken) {
    CURL* curl = curl_easy_init();
    std::string response;
    SessionResponse res;
    if(curl) {
        std::string url = BASE_URL + "/api/session/init";
        json j = {{"deviceToken", deviceToken}};
        std::string postData = j.dump();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                auto data = json::parse(response);
                res.sessionId = data["sessionId"];
                res.authUrl = data["authUrl"];
            } catch(...) {}
        }
        curl_easy_cleanup(curl);
    }
    return res;
}

SessionResponse Network::checkStatus(const std::string& deviceToken) {
    CURL* curl = curl_easy_init();
    std::string response;
    SessionResponse res;
    if(curl) {
        std::string url = BASE_URL + "/api/session/status?deviceToken=" + deviceToken;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                auto data = json::parse(response);
                res.status = data["status"];
                if (data.contains("user") && !data["user"].is_null()) {
                    res.userName = data["user"]["name"];
                }
            } catch(...) {}
        }
        curl_easy_cleanup(curl);
    }
    return res;
}

SDL_Texture* Network::downloadImage(const std::string& url) {
    if (url.empty()) return nullptr;
    CURL* curl = curl_easy_init();
    std::string response;
    SDL_Texture* texture = nullptr;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if(curl_easy_perform(curl) == CURLE_OK) {
            texture = Graphics::loadTextureFromMem(response.data(), response.size());
        }
        curl_easy_cleanup(curl);
    }
    return texture;
}

std::vector<DriveFile> Network::listDriveFiles(const std::string& userId, const std::string& folderId) {
    CURL* curl = curl_easy_init();
    std::string response;
    std::vector<DriveFile> files;
    if(curl) {
        std::string url = BASE_URL + "/api/drive/list?userId=" + userId + "&folderId=" + folderId;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                auto data = json::parse(response);
                for (auto& item : data["files"]) {
                    DriveFile f;
                    f.id = item["id"];
                    f.name = item["name"];
                    f.isFolder = (item["type"] == "folder");
                    if (item.contains("coverUrl") && !item["coverUrl"].is_null()) {
                        f.coverUrl = item["coverUrl"];
                    }
                    if (item.contains("system") && !item["system"].is_null()) {
                        f.system = item["system"];
                    }
                    files.push_back(f);
                }
            } catch(...) {}
        }
        curl_easy_cleanup(curl);
    }
    return files;
}
