#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include <switch.h>
#include <string>
#include <vector>
#include <SDL.h>

struct GameInfo {
    uint64_t titleId;
    std::string name;
    SDL_Texture* icon;
};

class Discovery {
public:
    static std::vector<GameInfo> listGames();
    static void cleanup(std::vector<GameInfo>& games);

private:
    static SDL_Texture* getIcon(uint64_t titleId);
};

#endif
