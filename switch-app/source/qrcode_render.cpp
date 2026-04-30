#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "qrcodegen.hpp"
#include <string>

using namespace qrcodegen;

// Renderiza QR code em SDL_Surface
SDL_Surface* renderQRCode(const std::string& text, int scale = 10) {
    // Gera QR code
    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);
    
    int size = qr.getSize();
    int margin = 2;
    int qrSize = (size + margin * 2) * scale;
    
    // Cria surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, qrSize, qrSize, 32, 
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    
    if (!surface) return nullptr;
    
    // Preenche com branco
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));
    
    // Renderiza módulos do QR code
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qr.getModule(x, y)) {
                // Módulo preto
                SDL_Rect rect;
                rect.x = (x + margin) * scale;
                rect.y = (y + margin) * scale;
                rect.w = scale;
                rect.h = scale;
                
                SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0));
            }
        }
    }
    
    return surface;
}
