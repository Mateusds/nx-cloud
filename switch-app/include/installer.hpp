#ifndef INSTALLER_HPP
#define INSTALLER_HPP

#include <switch.h>
#include <string>

class Installer {
public:
    static bool installFromUrl(const std::string& url, const std::string& name);
    static bool installForwarder(const std::string& romUrl, const std::string& romName, const std::string& system);
    static void setProgress(float p) { progress = p; }
    static float getProgress() { return progress; }

private:
    static float progress;
};

#endif
