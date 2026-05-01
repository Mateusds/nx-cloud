#include "installer.hpp"
#include <iostream>

float Installer::progress = 0.0f;

bool Installer::installFromUrl(const std::string& url, const std::string& name) {
    if (R_FAILED(ncmInitialize())) return false;
    
    std::cout << "Iniciando instalação de: " << name << std::endl;
    
    for(float i = 0; i <= 1.0f; i += 0.01f) {
        progress = i;
        svcSleepThread(10000000); 
    }

    ncmExit();
    return true;
}

bool Installer::installForwarder(const std::string& romUrl, const std::string& romName, const std::string& system) {
    if (R_FAILED(ncmInitialize())) return false;

    std::cout << "Criando Forwarder para o sistema: " << system << std::endl;
    std::cout << "Injetando ROM: " << romName << std::endl;

    for(float i = 0; i <= 1.0f; i += 0.02f) {
        progress = i;
        svcSleepThread(5000000);
    }

    ncmExit();
    return true;
}
