#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <cassert>
#include <functional>
#include <unordered_map>

class MGApplication;
class MGMainWindow;
class MGWindow;
class MGWidget;


bool isInsideRect(const SDL_Rect& rect, const int x, const int y);
bool isMouseEvent(const SDL_Event &event);
SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer);

class SignalManager {
friend class MGApplication;

    std::unordered_map<std::string, std::vector<std::function<void()>>> signals;

private:
    SignalManager() = default;
    ~SignalManager() = default;

public:
    void connect(const std::string &singnalString, std::function<void()> slotFunction) {
        signals[singnalString].push_back(slotFunction);
    }
    
    void emit(const std::string &singnalString) const {
        auto it = signals.find(singnalString);
        if (it == signals.end()) return;
        for (auto& slotFunction : it->second) {
            slotFunction();
        }
    }
};

class MGWidget {
friend class MGWindow;

protected:
    SignalManager *signalManager_ = nullptr;
    SDL_Rect viewport_ = {};
    SDL_Point inWindowPos_ = {};

    const MGWindow *parent_ = nullptr;

    int width_ = 0;
    int height_ = 0;
    MGWidget(const int width, const int height, const MGWindow *parent=nullptr): width_(width), height_(height), parent_(parent) {};
    ~MGWidget() = default;

private:
    void updateViewport(const SDL_Point &newWindowOffset) {
        viewport_.x = inWindowPos_.x + newWindowOffset.x;
        viewport_.y = inWindowPos_.y + newWindowOffset.y;
    }
    void setPosition(const SDL_Point &windowOffset, const SDL_Point &inWindowPos) {
        inWindowPos_ = inWindowPos;
        viewport_ = {inWindowPos.x + windowOffset.x, inWindowPos.y + windowOffset.y, width_, height_};
    }

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

    void setSignalManager(const SignalManager *const signalManager) {
        signalManager_ = signalManager;
    }
    void emitSignal(const std::string &singnalString) {
        signalManager_->emit(singnalString);
    }
};

class MGButton : public MGWidget {
friend class MGWindow;
    const char *pressedButtonTexturePath_ = nullptr;
    const char *unpressedButtonTexturePath_ = nullptr;

    SDL_Texture* pressedButtonTexture_ = nullptr;
    SDL_Texture* unpressedButtonTexture_ = nullptr;
    bool textureCreated = false;
   
    bool pressed_ = false;
    std::function<void()> onClickFunction_= nullptr;
    const MGWindow *parent_=nullptr;

public:
    MGButton
    (
        const int width, const int height,
        const char *pressedButtonTexturePath, const char *unpressedButtonTexturePath, 
        std::function<void()> onClickFunction=nullptr, const MGWindow *parent=nullptr
    ): 
        MGWidget(width, height, parent),  
        pressedButtonTexturePath_(pressedButtonTexturePath),
        unpressedButtonTexturePath_(unpressedButtonTexturePath),
        onClickFunction_(onClickFunction)
    {}
 
private:    
    ~MGButton() {
        if (pressedButtonTexture_) SDL_DestroyTexture(pressedButtonTexture_);
        if (unpressedButtonTexture_) SDL_DestroyTexture(unpressedButtonTexture_);
    }

    void initTexture(SDL_Renderer* renderer) {
        assert(renderer);
    
        if (textureCreated) return;
        textureCreated = true;
    
        pressedButtonTexture_ = createTexture(pressedButtonTexturePath_, renderer);    
        unpressedButtonTexture_ = createTexture(unpressedButtonTexturePath_, renderer);    
    }

    void paintEvent(SDL_Renderer* renderer) override {
        initTexture(renderer);
    
        if (pressed_) {
            setPressedTexture(renderer);
        } else {
            setUnPressedTexture(renderer);
        }
    }

    void setPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, width_, height_};
        SDL_RenderCopy(renderer, pressedButtonTexture_, NULL, &buttonRect);
    }

    void setUnPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, width_, height_};
        SDL_RenderCopy(renderer, unpressedButtonTexture_, NULL, &buttonRect);  
    }

    void handleEvent(const SDL_Event &event) override {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                pressed_ = true;
                if (onClickFunction_) onClickFunction_();
                break;
            case SDL_MOUSEBUTTONUP:
                pressed_ = false;
                break;
            default:
                break;
        }
    }
};

// class MGCanvas : public MGWidget {
// friend class MGWindow;
// private:
//     MGCanvas(SignalManager *signalManager, const SDL_Point &inWindowPos, const int width, const int height, const SDL_Point &windowOffset):
//         MGWidget(signalManager, inWindowPos, width, height, windowOffset) {}
// };

class MGWindow {
friend class MGMainWindow;

    SignalManager *signalManager_ = nullptr;

    SDL_Rect viewport_ = {};
    int width_ = 0;
    int height_ = 0;

    SDL_Point draggingStateDelta = {};
    bool draggingState = false;

    bool wasUpdated = false;

    std::vector<MGWidget *> widgets_ = {};

private:
    MGWindow();
    MGWindow(const SignalManager *signalManager, const int x, const int y, const int width, const int height):
    width_(width), height_(height), signalManager_(signalManager)
    {
        viewport_ = {x, y, width, height};
    }

    ~MGWindow() {
        for (auto widget : widgets_) {
            delete widget;
        }
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
            draggingStateDelta.x += event.motion.xrel;
            draggingStateDelta.y += event.motion.yrel;
            draggingState = true;
        }
    }

    void dragWindow() {
        viewport_.x += draggingStateDelta.x;
        viewport_.y += draggingStateDelta.y;
        for (auto widget : widgets_) {
            widget->updateViewport({viewport_.x, viewport_.y});
        }
        draggingState = false;
        draggingStateDelta = {0, 0};
    }

    void update() {
        if (draggingState) {
            dragWindow();
            wasUpdated = true;
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
    void addWidget(MGWidget *const widget, const SDL_Point &position) {
        assert(widget);

        if (widget->parent_ != nullptr) {
            std::cerr << "Widget can't be added : parent parent already exists\n";
            return;
        }

        widgets_.push_back((MGWidget *) widget);
        widgets_.back()->setSignalManager(signalManager_);
        widgets_.back()->setPosition({viewport_.x, viewport_.y}, position);
    }
};

class MGMainWindow {
friend class MGApplication;

    SignalManager *signalManager_=nullptr;
    SDL_Window* window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    int height = 0;
    int width = 0;

    std::vector<MGWindow *> windows_ = {};

public:
    MGWindow *addWindow(const int x, const int y, const int width, const int height) {
        MGWindow *newWindow = new MGWindow(signalManager_, x, y, width, height);
        windows_.push_back(newWindow);

        return windows_.back();
    }

private:
    MGMainWindow() {};

    MGMainWindow
    (
        SignalManager *signalManager,
        const char *windowTitle,
        const int width,
        const int height
    ) : 
        signalManager_(signalManager)
    {
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

        for (auto window : windows_) {
            delete window;
        }
    }
    

    void handleEvent(const SDL_Event &event) {
        int mouseX = 0, mouseY = 0;
    
        if (isMouseEvent(event)) {
            SDL_GetMouseState(&mouseX, &mouseY);
            for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
                if ((*it)->isInside(mouseX, mouseY)) {
                    (*it)->handleEvent(event); 
                    break; // only one window can handle mouse event
                }
            }
        }
    }
    
    void reorderWindows() {
        int curWindowIdx = windows_.size() - 1;
        while (curWindowIdx - 1 >= 0) {
            MGWindow *leftWindow = windows_[curWindowIdx - 1];
            MGWindow *rightWindow = windows_[curWindowIdx];
            if (leftWindow->wasUpdated && !rightWindow->wasUpdated) {
                std::swap(windows_[curWindowIdx - 1], windows_[curWindowIdx]);
            }
            curWindowIdx--;
        }

        for (auto window : windows_) {
            window->wasUpdated = false;
        }
    }

    void update() {
        for (auto window : windows_) {
            window->update();
        }

        reorderWindows();
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
    SignalManager signalManager;

    MGMainWindow *mainWindow_ = nullptr;
    
    const Uint32 frameDelay = 16;
    std::vector<std::function<void()>> userEvents_;

public:
    MGApplication(): signalManager() {}
    
    ~MGApplication() {
        if (mainWindow_) delete mainWindow_; 
        SDL_Quit();
    }

    void setMainWindow(const char *windowTitle, const int width, const int height) {
        assert(windowTitle);
        if (IMG_Init(IMG_INIT_PNG) == 0) throw std::runtime_error(std::string("SDL_Init failed") + SDL_GetError());

        if (SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(std::string("SDL_Init failed") + SDL_GetError());
    
        try {
            if (!mainWindow_) mainWindow_ = new MGMainWindow(&signalManager, windowTitle, width, height);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("setMainWindow failed"));
        }
    }

    MGMainWindow *getMainWindow() { return mainWindow_; }
    SignalManager *getSignalManager() { return &signalManager; }

    void addEventToMainLoop(std::function<void()> userEvent) {
        userEvents_.push_back(userEvent);
    }

    void run() {
        bool running = true;
        while (running) {
            Uint32 frameStart =  SDL_GetTicks();

            handleEvents(&running);
            mainWindow_->update();
            
            for (auto userEvent : userEvents_) {
                userEvent();
            }
            
            mainWindow_->render();

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        }
    }

private:
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

};


#endif // MGAPPLICATION_H