#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <SDL2/SDL.h>
#include <stdexcept>
#include <memory>
#include <iostream>

class MGMainWindow {
    SDL_Window* window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    MGMainWindow() {};

    MGMainWindow
    (
        const char *windowTitle,
        const int width,
        const int height
    ) {
        window_ = SDL_CreateWindow(
            windowTitle,     
            SDL_WINDOWPOS_CENTERED,        
            SDL_WINDOWPOS_CENTERED,        
            width,                            
            height,
            0);
        if (!window_) throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
        }
    }

    ~MGMainWindow() {
        if (window_) SDL_DestroyWindow(window_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
    }

    void render() {
        SDL_RenderPresent(renderer_);
    }

    friend class MGApplication;
};


class MGApplication {
    MGMainWindow *mainWindow_ = nullptr;

    // SignalManager signalManager;

public:
    MGApplication() {}
    
    ~MGApplication() {
        SDL_Quit();
        if (mainWindow_) delete mainWindow_; 
    }

    void setMainWindow(const char *windowTitle, const int width, const int height) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(std::string("SDL_Init failed") + SDL_GetError());
    
        try {
            if (!mainWindow_) mainWindow_ = new MGMainWindow(windowTitle, width, height);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("setMainWindow failed"));
        }
    }

    void run() {
        SDL_Event e;
        bool running = true;
        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            }

            // SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
            // SDL_RenderClear(ren);

            // /* simple white rectangle in center */
            // SDL_Rect r = { 220, 140, 200, 200 };
            // SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            // SDL_RenderFillRect(ren, &r);

            mainWindow_->render();
        }
    }
};


#endif // MGAPPLICATION_H