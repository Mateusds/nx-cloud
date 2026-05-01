#include "graphics.hpp"
#include <switch.h>

SDL_Window* Graphics::window = nullptr;
SDL_Renderer* Graphics::renderer = nullptr;
// TTF_Font* Graphics::font = nullptr;

bool Graphics::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) return false;
    // if (TTF_Init() < 0) return false;
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) < 0) return false;

    window = SDL_CreateWindow("NX-Cloud", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    return true;
}

void Graphics::exit() {
    // if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    IMG_Quit();
    // TTF_Quit();
    SDL_Quit();
}

void Graphics::clear() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
    SDL_RenderClear(renderer);
}

void Graphics::present() {
    SDL_RenderPresent(renderer);
}

void Graphics::drawRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Graphics::drawText(const std::string& text, int x, int y, int size, SDL_Color color) {
    // Temporariamente desativado até implementarmos alternativa ao SDL_ttf
}

void Graphics::drawImage(SDL_Texture* texture, int x, int y, int w, int h) {
    if (!texture) return;
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

#include "qrcodegen.hpp"

void Graphics::drawQRCode(const std::string& text, int x, int y, int size) {
    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);
    int qrSize = qr.getSize();
    float scale = (float)size / qrSize;

    for (int iy = 0; iy < qrSize; iy++) {
        for (int ix = 0; ix < qrSize; ix++) {
            if (qr.getModule(ix, iy)) {
                drawRect(x + (ix * scale), y + (iy * scale), scale + 1, scale + 1, {255, 255, 255, 255});
            } else {
                drawRect(x + (ix * scale), y + (iy * scale), scale + 1, scale + 1, {0, 0, 0, 255});
            }
        }
    }
}

SDL_Texture* Graphics::loadTextureFromMem(const void* buffer, size_t size) {
    SDL_RWops* rw = SDL_RWFromMem((void*)buffer, size);
    SDL_Texture* tex = IMG_LoadTexture_RW(renderer, rw, 1);
    return tex;
}
