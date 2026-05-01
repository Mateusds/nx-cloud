#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL.h>
#include <SDL_image.h>
// #include <SDL2/SDL_ttf.h> (Removido temporariamente)
#include <string>
#include <vector>

class Graphics {
public:
    static bool init();
    static void exit();
    
    static void clear();
    static void present();
    
    static void drawRect(int x, int y, int w, int h, SDL_Color color);
    static void drawText(const std::string& text, int x, int y, int size, SDL_Color color);
    static void drawImage(SDL_Texture* texture, int x, int y, int w, int h);
    static void drawQRCode(const std::string& text, int x, int y, int size);
    
    static SDL_Texture* loadTextureFromMem(const void* buffer, size_t size);
    
    static SDL_Renderer* getRenderer() { return renderer; }

private:
    static SDL_Window* window;
    static SDL_Renderer* renderer;
    // static TTF_Font* font;
};

#endif
