#pragma once
#include <SDL2/SDL.h>
#include <string>

SDL_Surface* renderQRCode(const std::string& text, int scale = 10);
