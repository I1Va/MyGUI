#ifndef MyGUI_H
#define MyGUI_H

#include <vector>
#include <functional>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"
#include "Events.h"


SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer);


class Widget;

class RendererGuard {
    SDL_Renderer* renderer_;
    SDL_Texture* savedTarget_;
    SDL_Rect savedViewport_;
    SDL_Rect savedClip_;
    Uint8 r_, g_, b_, a_;
    SDL_BlendMode blend_;

public:
    RendererGuard(SDL_Renderer* renderer);
    ~RendererGuard();
};

struct Rect : public SDL_Rect {
    Rect(int x = 0, int y = 0, int w = 0, int h = 0);
    gm_dot<int, 2> pos() const;
};

bool isInsideRect(const Rect& rect, const int x, const int y);

struct UIManagerglobalState {
    const Widget *hovered = nullptr;
    const Widget *mouseActived = nullptr;
};

class UIManager {
    Widget *wTreeRoot_ = nullptr;
    UIManagerglobalState glState_;
    Uint32 frameDelay_;

    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *mainWindow_ = nullptr;

private:
    void globalStateOnMouseMove(const Widget *wgt, const MouseMotionEvent &event);
    void globalStateOnMouseDown(const Widget *wgt, const MouseButtonEvent &event);
    void handleSDLEvents(bool *running);
    void initWTree(Widget *wgt);

public: // user API
    UIManager(int width, int height, Uint32 frameDelay = 32);
    ~UIManager();

    void setMainWidget(Widget *mainWidget);
    void run();
    const Widget *hovered() const { return glState_.hovered; }
    const Widget *mouseActived() const { return glState_.mouseActived; }

friend class Widget;
};

class Widget {
protected:
    UIManager *UIManager_ = nullptr;
    Widget *parent_ = nullptr;

    Rect rect_;
    SDL_Texture* texture_ = nullptr;

    bool needRerender_ = true;

public:
    Widget(int x, int y, int width, int height, Widget *parent = nullptr);
    virtual ~Widget();


    virtual void renderSelfAction(SDL_Renderer* renderer);
    virtual bool render(SDL_Renderer* renderer);
    virtual bool update();
    virtual bool updateSelfAction();

    // Events: return false to stop propagation (CONSUME), true to continue (PROPAGATE)
    // propagation logic
    virtual bool onMouseDown(const MouseButtonEvent &event);
    virtual bool onMouseUp(const MouseButtonEvent &event);
    virtual bool onMouseMove(const MouseMotionEvent &event);

    // event self processing logic
    virtual bool onMouseDownSelfAction(const MouseButtonEvent &event);
    virtual bool onMouseUpSelfAction(const MouseButtonEvent &event);
    virtual bool onMouseMoveSelfAction(const MouseMotionEvent &event);

    // Getters / Setters
    virtual const std::vector<Widget *> &getChildren() const;
    void setRerenderFlag() { 
        needRerender_ = true;
        if (parent_) parent_->invalidate();
    }   
    void invalidate() { needRerender_ = true; }
    bool needRerender() const { return needRerender_; }
    Rect rect() const;
    const Widget *parent() const;
    SDL_Texture* texture();

friend class UIManager;
friend class Container;
};

class Container : public Widget {
protected:
    std::vector<Widget *> children_;
     
public:
    Container(int x, int y, int width, int height, Widget *parent=nullptr);
    ~Container();

    // Events
    bool onMouseDown(const MouseButtonEvent &event) override;
    bool onMouseUp(const MouseButtonEvent &event) override;
    bool onMouseMove(const MouseMotionEvent &event) override;

    // Stages
    bool update() override;
    bool render(SDL_Renderer* renderer) override;

    // Getters / setters
    const std::vector<Widget *> &getChildren() const override;

    // User API
    void addWidget(Widget *widget);
};

class Window : public Container {
protected:
    gm_dot<int, 2> accumulatedRel_ = {0, 0};
    bool replaced_ = true;
public:
    Window(int x, int y, int w, int h, Widget *parent=nullptr) : Container(x, y, w, h, parent) {}    

    bool onMouseMoveSelfAction(const MouseMotionEvent &event) override;
    bool updateSelfAction() override;
};

// class Button : public Widget {
    
// public:
//     Button(int x, int y, int w, int h) : Widget(x, y, w, h) {}    

//     void renderSelfAction(SDL_Renderer* renderer) {
//         assert(renderer);

//         SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
//         SDL_Rect full = {0, 0, rect_.w, rect_.h};
//         SDL_RenderFillRect(renderer, &full);
//     }
// };


class Button : public Widget {
    const char *pressedButtonTexturePath_ = nullptr;
    const char *unpressedButtonTexturePath_ = nullptr;

    SDL_Texture* pressedButtonTexture_ = nullptr;
    SDL_Texture* unpressedButtonTexture_ = nullptr;
    bool textureCreated = false;
   
    bool pressed_ = false;
    std::function<void()> onClickFunction_= nullptr;

public:
    Button
    (
        int x, int y, int w, int h,
        const char *unpressedButtonTexturePath, const char *pressedButtonTexturePath,
        std::function<void()> onClickFunction=nullptr, Widget *parent=nullptr
    ): 
        Widget(x, y, w, h, parent),  
        pressedButtonTexturePath_(pressedButtonTexturePath),
        unpressedButtonTexturePath_(unpressedButtonTexturePath),
        onClickFunction_(onClickFunction)
    {}
 
private:    
    ~Button() {
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

    void renderSelfAction(SDL_Renderer* renderer) override {
        initTexture(renderer);

        if (pressed_) {
            setPressedTexture(renderer);
        } else {
            setUnPressedTexture(renderer);
        }
    }

    void setPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_RenderCopy(renderer, pressedButtonTexture_, NULL, &buttonRect);
    }

    void setUnPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_RenderCopy(renderer, unpressedButtonTexture_, NULL, &buttonRect);  
    }

    bool onMouseUpSelfAction(const MouseButtonEvent &event) override {
        if (event.button == SDL_BUTTON_LEFT) {
            pressed_ = false;
            setRerenderFlag();
            return true;
        }
        return false;
    }

    bool onMouseDownSelfAction(const MouseButtonEvent &event) override {
        if (event.button == SDL_BUTTON_LEFT) {
            pressed_ = true;
            setRerenderFlag();
            if (onClickFunction_) onClickFunction_();
            return true;
        }
        return false;
    }
};

#endif // MyGUI_H
