#include "discovery.hpp"
#include "graphics.hpp"
#include <switch.h>
#include <algorithm>

std::vector<GameInfo> Discovery::listGames() {
    std::vector<GameInfo> games;
    NsApplicationRecord records[100];
    int total = 0;

    if (R_SUCCEEDED(nsListApplicationRecord(records, 100, 0, &total))) {
        for (int i = 0; i < total; i++) {
            GameInfo game;
            game.titleId = records[i].application_id;

            NsApplicationControlData controlData;
            u64 size = 0;
            if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, game.titleId, &controlData, sizeof(controlData), &size))) {
                NacpLanguageEntry* langEntry = nullptr;
                if (R_SUCCEEDED(nsGetApplicationDesiredLanguage(&controlData.nacp, &langEntry)) && langEntry != nullptr && langEntry->name[0] != '\0') {
                    game.name = langEntry->name;
                } else {
                    game.name = "Jogo Desconhecido";
                }
                game.icon = Graphics::loadTextureFromMem(controlData.icon, sizeof(controlData.icon));
            } else {
                game.name = "Jogo Desconhecido";
                game.icon = nullptr;
            }
            
            games.push_back(game);
        }
    }

    return games;
}

void Discovery::cleanup(std::vector<GameInfo>& games) {
    for (auto& game : games) {
        if (game.icon) SDL_DestroyTexture(game.icon);
    }
    games.clear();
}
