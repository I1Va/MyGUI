#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <SDL2/SDL.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <cassert>
#include <functional>







inline bool isInsideRect(const SDL_Rect& rect, const int x, const int y) {
    return x >= rect.x && x <= (rect.x + rect.w) && y >= rect.y && y <= rect.y + rect.h;
}

inline bool isMouseEvent(const SDL_Event &event) {
    return  event.type == SDL_MOUSEMOTION ||
            event.type == SDL_MOUSEBUTTONDOWN || 
            event.type == SDL_MOUSEBUTTONUP || 
            event.type == SDL_MOUSEWHEEL;
}

class MGWidget {
friend class MGWindow;

protected:
    SDL_Rect viewport_ = {};
    SDL_Point inWindowPos_ = {};

    int width_;
    int height_;
    
    void updateViewport(const SDL_Point &newWindowOffset) {
        viewport_.x = inWindowPos_.x + newWindowOffset.x;
        viewport_.y = inWindowPos_.y + newWindowOffset.y;
    }

    MGWidget(const SDL_Point &inWindowPos, const int width, const int height, const SDL_Point &windowOffset): 
        width_(width), height_(height), inWindowPos_(inWindowPos)
    {
        viewport_ = {inWindowPos.x + windowOffset.x, inWindowPos.y + windowOffset.y, width, height};
    }

    ~MGWidget() = default;

private:
    virtual void paintEvent(SDL_Renderer* renderer) {}


    bool isInside(const int x, const int y) {
        return isInsideRect(viewport_, x, y);
    }

    virtual void handleEvent(const SDL_Event &event) {}

    virtual void update() {}

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

        paintEvent(renderer);

        SDL_RenderSetViewport(renderer, &prevViewPort);
        SDL_RenderSetClipRect(renderer, &prevClip);
        SDL_SetRenderDrawColor(renderer, prev_r, prev_g, prev_b, prev_a);
    }

    // TODO: add other virtual methods: handleEvent, update, etc.
    
};


class MGButton : public MGWidget {
friend class MGWindow;
    bool pressed_ = false;
    std::function<void()> onClick;

private:
    MGButton(const SDL_Point &inWindowPos, const int width, const int height, const SDL_Point &windowOffset):
        MGWidget(inWindowPos, width, height, windowOffset) {}

    void paintEvent(SDL_Renderer* renderer) override {
        if (pressed_) {
            setPressedTexture(renderer);
        } else {
            setUnPressedTexture(renderer);
        }
    }

    void setPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, width_, height_};
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &buttonRect);    
    }

    void setUnPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, width_, height_};
        
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &buttonRect);    
    }

    void handleEvent(const SDL_Event &event) override {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                pressed_ = true;
                if (onClick) onClick();
                break;
            case SDL_MOUSEBUTTONUP:
                pressed_ = false;
                break;
            default:
                break;
        }
    }

};



class MGCanvas : public MGWidget {
friend class MGWindow;

private:
    void paintEvent(SDL_Renderer* renderer) override {
        SDL_Rect widgetRect = {0, 0, width_, height_};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // MGCanvas BACKGROUND COLOR
        SDL_RenderFillRect(renderer, &widgetRect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderDrawLine(renderer, 0, 0, 1000, 1000);
    }

    MGCanvas(const SDL_Point &inWindowPos, const int width, const int height, const SDL_Point &windowOffset):
        MGWidget(inWindowPos, width, height, windowOffset) {}
};


class MGWindow {
friend class MGMainWindow;
    SDL_Rect viewport_;
    int width_ = 0;
    int height_ = 0;

    SDL_Point moveDelta = {};
    bool moved = false;

    std::vector<MGWidget *> widgets_;


private:
    MGWindow();
    MGWindow(const int x, const int y, const int width, const int height):
    width_(width), height_(height) 
    {
        viewport_ = {x, y, width, height};
    }

    bool isInside(const int x, const int y) {
        return isInsideRect(viewport_, x, y);
    }

    void handleEvent(const SDL_Event &event) {
        int mouseX = 0, mouseY = 0;
        bool mouseOnFreeSpace = true;

        if (isMouseEvent(event)) {
            SDL_GetMouseState(&mouseX, &mouseY);
            for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
                if ((*it)->isInside(mouseX, mouseY)) {
                    (*it)->handleEvent(event);
                    mouseOnFreeSpace = false;
                }
            }
        }
        
        Uint32 mouseButtonState = event.motion.state;
        if (mouseOnFreeSpace && event.type == SDL_MOUSEMOTION && (mouseButtonState & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            moveDelta.x += event.motion.xrel;
            moveDelta.y += event.motion.yrel;
            moved = true;
        }
    }

    void update() {
        if (moved) {
            viewport_.x += moveDelta.x;
            viewport_.y += moveDelta.y;
            for (auto widget : widgets_) {
                widget->updateViewport({viewport_.x, viewport_.y});
            }
            moved = false;
            moveDelta = {0, 0};
        }
        for (auto widget : widgets_) {
            widget->update();
        }
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

        SDL_Rect widgetRect = {0, 0, width_, height_};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // WINDOW BACKGROUND COLOR
        SDL_RenderFillRect(renderer, &widgetRect);
    
        for (auto widget : widgets_) {
            widget->render(renderer); // paint events
        }

        SDL_RenderSetViewport(renderer, &prevViewPort);
        SDL_RenderSetClipRect(renderer, &prevClip);
        SDL_SetRenderDrawColor(renderer, prev_r, prev_g, prev_b, prev_a);
    }

public:
    void addCanvas(const int x, const int y, const int width, const int height) {
        MGCanvas *canvas = new MGCanvas({x, y}, std::min(width, viewport_.w - x), std::min(height, viewport_.h - y), {viewport_.x, viewport_.y});
        widgets_.push_back((MGWidget *) canvas);
    }

    void addButton(const int x, const int y, const int width, const int height) {
        MGButton *button = new MGButton({x, y}, std::min(width, viewport_.w - x), std::min(height, viewport_.h - y), {viewport_.x, viewport_.y});
        widgets_.push_back((MGWidget *) button);
    }
};

class MGMainWindow {
friend class MGApplication;

    SDL_Window* window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    int height = 0;
    int width = 0;

    std::vector<MGWindow *> windows_;

public:
    MGWindow *addWindow(const int x, const int y, const int width, const int height) {
        MGWindow *newWindow = new MGWindow(x, y, width, height);
        windows_.push_back(newWindow);

        return windows_.back();
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
    

    void handleEvent(const SDL_Event &event) {
        int mouseX = 0, mouseY = 0;
    
        if (isMouseEvent(event)) {
            SDL_GetMouseState(&mouseX, &mouseY);
            for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
                if ((*it)->isInside(mouseX, mouseY)) {
                    (*it)->handleEvent(event);
                }
            }
        }
    }

    void update() {
        for (auto window : windows_) {
            window->update();
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
        SDL_RenderClear(renderer_);

        for (auto childWindow : windows_) {
            childWindow->render(renderer_);
        }

        SDL_RenderPresent(renderer_);
    }
   
    
};

class MGApplication {
    MGMainWindow *mainWindow_ = nullptr;
    const Uint32 frameDelay = 16;
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

    void handleEvents(bool *running) {
        assert(running);
    
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                *running = false;
                break;
            }
            
            mainWindow_->handleEvent(event);
        }
    }

    void run() {
        bool running = true;
        while (running) {
            Uint32 frameStart =  SDL_GetTicks();
        
            handleEvents(&running);
            mainWindow_->update();
            mainWindow_->render();

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        }
    }
};


#endif // MGAPPLICATION_H