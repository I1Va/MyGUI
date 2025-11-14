#include "Common.h"
#include <SDL_image.h>

SDL_Rect getTextSize(TTF_Font *font, const char text[]) {
    SDL_Rect textRect = {0, 0, 0, 0};
    if (TTF_SizeUTF8(font, text, &textRect.w, &textRect.h)) {
        SDL_Log("TTF_SizeUTF8 failed: %s", TTF_GetError());
    }

    return textRect;
}

SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer) {
    assert(renderer);

    if (!texturePath) return nullptr;

    SDL_Texture* resultTexture = nullptr;
    SDL_Surface* surface = IMG_Load(texturePath);
    if (!surface) std::cerr << "Texture load failed : " << texturePath << "\n";

    if (surface) {
        resultTexture = SDL_CreateTextureFromSurface(renderer, surface);
        assert(resultTexture);
        SDL_SetTextureBlendMode(resultTexture, SDL_BLENDMODE_BLEND);
    
        SDL_FreeSurface(surface);
    }

    return resultTexture;
}

Uint32 SDL2gfxColorToUint32(SDL_Color c) {
    return (static_cast<Uint32>(c.a) << 24) |  
           (static_cast<Uint32>(c.b) << 16) |  
           (static_cast<Uint32>(c.g) << 8)  | 
            static_cast<Uint32>(c.r);          
}

SDL_Color Uint32ToSDL2gfxColor(Uint32 value) {
    SDL_Color color;
    color.r = value & 0xFF;
    color.g = (value >> 8) & 0xFF;
    color.b = (value >> 16) & 0xFF;
    color.a = (value >> 24) & 0xFF;
    return color;
}

SDL_Texture* createFontTexture(TTF_Font* font, const char text[], SDL_Color textColor, SDL_Renderer* renderer) {
    assert(font);
    assert(renderer);
    assert(text);


    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    assert(textSurface);

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    assert(textTexture);

    SDL_SetTextureBlendMode(textTexture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(textSurface);

    return textTexture;
}
