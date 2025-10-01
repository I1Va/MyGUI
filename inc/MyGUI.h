#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <vector>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"
#include "Events.h"

class UIManager {};




class Widget {
    const UIManager *UIManager_ = nullptr;
    const Widget *parent_;
    std::vector<Widget *> children_;

    SDL_Rect rect_; // истинный прямоугольник

    SDL_Rect viewPort_; // та часть прямоугольника, которая будет отрендерена
    SDL_Texture* texture_;
    
    bool needsReRender_ = false;

    

protected:
// Events
    virtual bool onMouseDown(const MouseButtonEvent &event) {};
    virtual bool onMouseUp(const MouseButtonEvent &event) {};
// Main loop stages
    virtual void update() {};
    virtual void render() {};
// User defined paintEvent
    virtual void paintEvent() {};
    

public:
    Widget(int x, int y, int width, int height, const Widget *parent=nullptr): rect_({x, y, width, height}), parent_(parent) {}

friend class UIManager;
};

struct UIManagerglobalState {
    Widget *target = nullptr;
};

class UIManager {
    Widget *wTreeRoot_ = nullptr;
    UIManagerglobalState glState_;

    Uint32 frameDelay_ = 32;

    

private:
    void resetTarget() { glState_.target = nullptr; }
    void setTarget(Widget *newTarget) { glState_.target = newTarget; }

    void handleSDLEvents(bool *running) {
        SDL_Event SDLEvent;

         while (SDL_PollEvent(&SDLEvent)) {
            if (SDLEvent.type == SDL_QUIT || SDLEvent.type == SDL_KEYDOWN && SDLEvent.key.keysym.sym == SDLK_ESCAPE) {
                *running = false;
                break;
            }
            
            MouseButtonEvent MouseButtonEvent;

            switch (SDLEvent.type) {
            case SDL_MOUSEBUTTONDOWN:
                wTreeRoot_->onMouseDown({SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button});
                break;
            case SDL_MOUSEBUTTONUP:
                wTreeRoot_->onMouseUp({SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button});
                break;
            default:
                break;
        }
        }

        
    }

public:
    explicit UIManager(Uint32 frameDelay): frameDelay_(frameDelay) {}

    void run() {
        bool running = true;
        while (running) {
            Uint32 frameStart =  SDL_GetTicks();

            handleSDLEvents(&running);
            // mainWindow_->update();
            
            // for (auto userEvent : userEvents_) {
            //     userEvent(frameDelay);
            // }
            
            // wTreeRoot->render();

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay_ > frameTime) SDL_Delay(frameDelay_ - frameTime);
        }
    }

friend class Widget;

};



#endif // MGAPPLICATION_H