#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <SDL2/SDL.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <cassert>


class MGWindow {
    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;

    MGWindow();
    MGWindow(const int x, const int y, const int width, const int height): x_(x), y_(y), width_(width), height_(height) {}

    void render(SDL_Renderer* renderer) {
        assert(renderer);
    
        SDL_Rect prevClip;
        SDL_RenderGetClipRect(renderer, &prevClip);

        SDL_Rect clipRect = { x_, y_, width_, height_ };
        SDL_RenderSetClipRect(renderer, &clipRect);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &clipRect);

        // Render child widgets — they draw within this clip
        // for (auto& widget : widgets_) {
        //     widget->render(renderer);
        // }
    
        SDL_RenderSetClipRect(renderer, &prevClip);
    }

    
    friend class MGMainWindow;
};


class MGMainWindow {
    SDL_Window* window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    int height = 0;
    int width = 0;

    std::vector<MGWindow *> windows;
    friend class MGApplication;

public:
    void addWindow(const int x, const int y, const int width, const int height) {
        MGWindow *newWindow = new MGWindow(x, y, width, height);
        windows.push_back(newWindow);
    }

private:
    MGMainWindow() {};

    MGMainWindow
    (
        const char *windowTitle,
        const int width,
        const int height
    ) {
        this->width;
        this->height;

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
        SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
        SDL_RenderClear(renderer_);

        for (auto childWindow : windows) {
            childWindow->render(renderer_);
        }

        SDL_RenderPresent(renderer_);
    }
   
    
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
        assert(windowTitle);
    
        if (SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(std::string("SDL_Init failed") + SDL_GetError());
    
        try {
            if (!mainWindow_) mainWindow_ = new MGMainWindow(windowTitle, width, height);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("setMainWindow failed"));
        }
    }

    MGMainWindow *mainWindow() { return mainWindow_; }

    void run() {
        SDL_Event e;
        bool running = true;
        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            }
            mainWindow_->render();
        }
    }
};


#endif // MGAPPLICATION_H