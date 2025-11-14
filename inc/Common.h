#ifndef COMMON_H
#define COMMON_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gm_primitives.hpp"


inline constexpr int WINDOW_BORDER_SIZE = 15;
inline constexpr SDL_Color DEFAULT_WINDOW_COLOR = {220, 220, 220, 255};
inline constexpr SDL_Color BLACK_SDL_COLOR = {0, 0, 0, 255};
inline constexpr SDL_Color RED_SDL_COLOR = {255, 0, 0, 255};
inline constexpr SDL_Color BLUE_SDL_COLOR = {0, 0, 255, 255};
inline constexpr SDL_Color WHITE_SDL_COLOR = {255, 255, 255, 255};

SDL_Rect getTextSize(TTF_Font *font, const char text[]);
SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer);
SDL_Texture* createFontTexture(TTF_Font* font, const char text[], SDL_Color textColor, SDL_Renderer* renderer);
Uint32 SDL2gfxColorToUint32(SDL_Color color);
SDL_Color Uint32ToSDL2gfxColor(Uint32 value);

struct Rect : public SDL_Rect {
    Rect(int x = 0, int y = 0, int w = 0, int h = 0) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
    gm_dot<int, 2> pos() {
        return { x, y };
    }
};

inline bool isNullRect(const SDL_Rect &rect) {
    return (rect.x == 0 && rect.y == 0 &&
            rect.w == 0 && rect.h == 0);
}

inline bool isInsideRect(const Rect& rect, const int x, const int y) {
    return (x >= rect.x) && (x < rect.x + rect.w) && (y >= rect.y) && (y < rect.y + rect.h);
}

inline bool isMouseSDLevent(const SDL_Event &event) {
    return 
        event.type == SDL_MOUSEMOTION       || 
        event.type == SDL_MOUSEBUTTONDOWN   ||
        event.type == SDL_MOUSEBUTTONUP     ;
}

class RendererGuard {
    SDL_Renderer* renderer_;
    SDL_Texture* savedTarget_;
    SDL_Rect savedViewport_;
    SDL_Rect savedClip_;
    Uint8 r_, g_, b_, a_;
    SDL_BlendMode blend_;

public:
    RendererGuard(SDL_Renderer* renderer) {
        savedTarget_ = SDL_GetRenderTarget(renderer_);
        SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
        SDL_GetRenderDrawBlendMode(renderer_, &blend_);
        SDL_RenderGetViewport(renderer_, &savedViewport_);
        SDL_RenderGetClipRect(renderer_, &savedClip_);
    }

    ~RendererGuard() {
        if (renderer_)
        SDL_SetRenderTarget(renderer_, savedTarget_); // restore previous render target
            
        SDL_SetRenderDrawColor(renderer_, r_, g_, b_, a_); 
        SDL_SetRenderDrawBlendMode(renderer_, blend_);     
        
        if (!isNullRect(savedViewport_))
            SDL_RenderSetViewport(renderer_, &savedViewport_);
        else
            SDL_RenderSetViewport(renderer_, nullptr);

        if (!isNullRect(savedClip_))
            SDL_RenderSetClipRect(renderer_, &savedClip_);
        else
            SDL_RenderSetClipRect(renderer_, nullptr);
    }
};

#endif // COMMON_H