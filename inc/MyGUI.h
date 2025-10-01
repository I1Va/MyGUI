#ifndef MGAPPLICATION_H
#define MGAPPLICATION_H

#include <vector>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"
#include "Events.h"

class UIManager;
class Widget;
class Container;



struct Rect : public SDL_Rect {
    Rect(int x, int y, int w, int h) : SDL_Rect(x, y, w, h) {}
    gm_dot<int, 2> pos() { return {x, y}; }
};
bool isInsideRect(const Rect& rect, const int x, const int y);


class RendererGuard {
    SDL_Renderer* renderer_;
    Uint8 r_, g_, b_, a_;
    SDL_BlendMode blend_;
    SDL_Rect viewport_;
    SDL_Rect clip_;

public:
    RendererGuard(SDL_Renderer* renderer) : renderer_(renderer) {
        SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
        SDL_GetRenderDrawBlendMode(renderer_, &blend_);
        SDL_RenderGetViewport(renderer_, &viewport_);
        SDL_RenderGetClipRect(renderer_, &clip_);
    }

    ~RendererGuard() {
        SDL_SetRenderDrawColor(renderer_, r_, g_, b_, a_);
        SDL_SetRenderDrawBlendMode(renderer_, blend_);
        SDL_RenderSetViewport(renderer_, &viewport_);
        SDL_RenderSetClipRect(renderer_, &clip_);
    }
};

class Widget {
protected:
    UIManager *UIManager_ = nullptr;
    const Widget *parent_;

    Rect rect_;

    SDL_Texture* texture_ = nullptr;

    bool needsReRender_ = false;    

private:
// Events
    virtual bool onMouseDown(const MouseButtonEvent &event) = 0;
    virtual bool onMouseUp(const MouseButtonEvent &event) = 0;
// Main loop stages
    virtual void update() = 0;
    virtual void render() = 0;
// User defined paintEvent
    virtual void paintEvent() = 0;
    
public:
    Widget(int x, int y, int width, int height, const Widget *parent=nullptr): 
        parent_(parent), rect_({x, y, width, height}) {}
    
    void setParent(const Widget *parent) { parent_ = parent; } 
    const Widget *parent() const { return parent_; }
    const SDL_Texture* texture() const { return texture_; }
    Rect rect() const { return rect_; }

friend class UIManager;
friend class Container;
};

class Container : public Widget {
    std::vector<Widget *> children_;

protected:
// Events
    bool onMouseDown(const MouseButtonEvent &event) override {
        if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

        for (Widget *child : children_) {
            MouseButtonEvent childLocal = event;
            childLocal.pos -= child->rect().pos();
            if (!child->onMouseDown(childLocal)) {
                return false; // stop event propagation
            }
        }

        return true; // continue event propagation
    };

    bool onMouseUp(const MouseButtonEvent &event) override {
        if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

        for (Widget *child : children_) {
            MouseButtonEvent childLocal = event;
            childLocal.pos -= child->rect().pos();
            if (!child->onMouseUp(childLocal)) {
                return false; // stop event propagation
            }
        }

        return true; // continue event propagation
    };

// Main loop stages
    void update() override {
        return;
    };

    void render() override {
        RendererGuard rendererGuard();

        SDL_Texture* texture_ = SDL_CreateTexture(UIManager_->renderer(),
                                                  SDL_PIXELFORMAT_RGBA8888,
                                                  SDL_TEXTUREACCESS_TARGET,
                                                  rect_.w, rect_.h);
        
        SDL_SetRenderTarget(UIManager_->renderer(), texture_);
        
        SDL_Rect clip = { 0, 0, rect_.w, rect_.h };
        SDL_RenderSetClipRect(UIManager_->renderer(), &clip);
                
        for (Widget *child : children_) {
            child->render();
            SDL_Rect chldRect = child->rect();
            SDL_RenderCopy(UIManager_->renderer(), child->texture_, NULL, &chldRect);
        }
    };

public:
    Container(int x, int y, int width, int height, const Widget *parent=nullptr): Widget(x, y, width, height, parent) {}
    ~Container() {
        for (Widget *child : children_)
            delete child;
    }

    void addWdiget(Widget *widget) {
        if (widget->parent() == this || widget->parent() == nullptr) {
            widget->setParent(this);
            children_.push_back(widget); 
        } else {
            std::cerr << "addWdiget failed : parent does not match\n";
        }
    }
};


struct UIManagerglobalState {
    Widget *target = nullptr;
};

class UIManager {
    Widget *wTreeRoot_ = nullptr;
    UIManagerglobalState glState_;

    SDL_Renderer* renderer_ = nullptr;

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

    void setMainWidget(Widget *mainWidget) {
        if (wTreeRoot_ != nullptr) {
            std::cerr << "setMainWidget failed : wTreeRoot != nullptr\n";
            return;
        }
    
        wTreeRoot_ = mainWidget;
    }


    SDL_Renderer* renderer() { return renderer_; }
    
    void run() {
        bool running = true;
        while (running) {
            Uint32 frameStart =  SDL_GetTicks();

            handleSDLEvents(&running);
            wTreeRoot_->update();
            
            // for (auto userEvent : userEvents_) {
            //     userEvent(frameDelay);
            // }
            
            wTreeRoot_->render();

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay_ > frameTime) SDL_Delay(frameDelay_ - frameTime);
        }
    }

friend class Widget;
};



#endif // MGAPPLICATION_H