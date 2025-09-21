#include "MyGUI.h"

bool isInsideRect(const SDL_Rect& rect, const int x, const int y) {
    return x >= rect.x && x <= (rect.x + rect.w) && y >= rect.y && y <= rect.y + rect.h;
}

bool isMouseEvent(const SDL_Event &event) {
    return  event.type == SDL_MOUSEMOTION ||
            event.type == SDL_MOUSEBUTTONDOWN || 
            event.type == SDL_MOUSEBUTTONUP || 
            event.type == SDL_MOUSEWHEEL;
}


SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer) {
    assert(renderer);

    if (!texturePath) return nullptr;

    SDL_Texture* resultTexture = nullptr;
    SDL_Surface* surface = IMG_Load(texturePath);
    if (!surface) std::cerr << "MGButton texture load failed : " << texturePath << "\n";

    if (surface) {
        resultTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    return resultTexture; 
}