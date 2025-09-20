#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <SDL2/SDL.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <cassert>











class MGWidget {
friend class MGWindow;

protected:
    SDL_Rect viewport_;
    
    MGWidget(const int x, const int y, const int width, const int height, const int windowOffsetX, const int windowOffsetY) {
        viewport_ = {x + windowOffsetX, y + windowOffsetY, width + windowOffsetX, height + windowOffsetY};
    }

    ~MGWidget() = default;

private:
    virtual void paintEvent(SDL_Renderer* renderer) {}

    void render(SDL_Renderer* renderer) {
        assert(renderer);
        
        SDL_Rect prevClip;
        SDL_RenderGetClipRect(renderer, &prevClip);
        SDL_Rect prevViewPort;
        SDL_RenderGetViewport(renderer, &prevViewPort);
        Uint8 prev_r, prev_g, prev_b, prev_a;
        SDL_GetRenderDrawColor(renderer, &prev_r, &prev_g, &prev_b, &prev_a);


        SDL_Rect clipRect = {0, 0, viewport_.w, viewport_.h};
        SDL_RenderSetViewport(renderer, &viewport_);
        SDL_RenderSetClipRect(renderer, &clipRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // WIDGET BACKGROUND COLOR
        SDL_RenderFillRect(renderer, &clipRect);

        paintEvent(renderer);


        SDL_RenderSetViewport(renderer, &prevViewPort);
        SDL_RenderSetClipRect(renderer, &prevClip);
        SDL_SetRenderDrawColor(renderer, prev_r, prev_g, prev_b, prev_a);
    }

    // TODO: add other virtual methods: handleEvent, update, etc.
    
};



class MGCanvas : public MGWidget {

private:
    void paintEvent(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderDrawLine(renderer, 0, 0, 1000, 1000);
    }

public:
    MGCanvas(const int x, const int y, const int width, const int height, const int windowOffsetX, const int windowOffsetY):
        MGWidget(x, y, width, height, windowOffsetX, windowOffsetY) {}
};




class MGWindow {
friend class MGMainWindow;
    SDL_Rect viewport_;

    std::vector<MGWidget *> widgets_;


private:
    MGWindow();
    MGWindow(const int x, const int y, const int width, const int height) {
        viewport_ = {x, y, width, height};
    }

    void render(SDL_Renderer* renderer) {
        assert(renderer);
        
        
        SDL_Rect prevClip;
        SDL_RenderGetClipRect(renderer, &prevClip);
        SDL_Rect prevViewPort;
        SDL_RenderGetViewport(renderer, &prevViewPort);
        Uint8 prev_r, prev_g, prev_b, prev_a;
        SDL_GetRenderDrawColor(renderer, &prev_r, &prev_g, &prev_b, &prev_a);

        SDL_Rect clipRect = {0, 0, viewport_.w, viewport_.h};
        SDL_RenderSetViewport(renderer, &viewport_);
        SDL_RenderSetClipRect(renderer, &clipRect);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // WINDOW BACKGROUND COLOR
        SDL_RenderFillRect(renderer, &clipRect);

        for (auto widget : widgets_) {
            widget->render(renderer); // paint event
        }

        SDL_RenderSetViewport(renderer, &prevViewPort);
        SDL_RenderSetClipRect(renderer, &prevClip);
        SDL_SetRenderDrawColor(renderer, prev_r, prev_g, prev_b, prev_a);
    }

public:
    void addCanvas(const int x, const int y, const int width, const int height) {
        MGCanvas *canvas = new MGCanvas(x, y, std::min(width, viewport_.w - x), std::min(height, viewport_.h - y), viewport_.x, viewport_.y);
        widgets_.push_back((MGWidget *) canvas);
    }
};

class MGMainWindow {
friend class MGApplication;

    SDL_Window* window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    int height = 0;
    int width = 0;

    std::vector<MGWindow *> windows;

public:
    MGWindow *addWindow(const int x, const int y, const int width, const int height) {
        MGWindow *newWindow = new MGWindow(x, y, width, height);
        windows.push_back(newWindow);

        return windows.back();
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
    MGApplication() = default;
    
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