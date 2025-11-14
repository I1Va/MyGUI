#ifndef MyGUI_H
#define MyGUI_H

#include <vector>
#include <functional>

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"
#include "Events.h"


inline constexpr int DEFAULT_FRAME_DELAY_MS = 1000 / 60;
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

struct ButtonTexturePath {
    const char *unpressed;
    const char *pressed;
};

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
    Widget *hovered = nullptr;
    Widget *mouseActived = nullptr;
};

class UIManager {
    Widget *wTreeRoot_   = nullptr;
    Widget *modalWidget_ = nullptr; 
    UIManagerglobalState glState_;
    Uint32 frameDelayMs_;

    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *mainWindow_ = nullptr;

    std::vector<std::function<void(int)>> userEvents_ = {};

private:
    void globalStateOnMouseWheel(Widget *wgt, const MouseWheelEvent  &event);
    void globalStateOnMouseMove (Widget *wgt, const MouseMotionEvent &event);
    void globalStateOnMouseDown (Widget *wgt, const MouseButtonEvent &event);
    void globalStateOnKeyDown   (Widget *wgt, const KeyEvent         &event);
    void globalStateOnKeyUp     (Widget *wgt, const KeyEvent         &event);

    void handleSDLEvents(bool *running);
    void initWTree(Widget *wgt);

public: // user API
    UIManager(int width, int height, Uint32 frameDelay=DEFAULT_FRAME_DELAY_MS);
    ~UIManager();

    void setMainWidget(int x, int y, Widget *mainWidget);
    void pinModalWidget(int x, int y, Widget *modalWidget);
    void unpinModalWidget();

    void run();
    const Widget *hovered() const { return glState_.hovered; }
    void setHovered(Widget *widget) { glState_.hovered = widget; }
    const Widget *mouseActived() const { return glState_.mouseActived; }
    void setMouseActived(Widget *widget) { glState_.mouseActived = widget; }

    void addUserEvent(std::function<void(int)> userEvent) { userEvents_.push_back(userEvent); };
    TTF_Font* createFont(const char fontPath[], const size_t fontSize);

friend class Widget;
};

class Widget {
protected:
    UIManager *UIManager_ = nullptr;
    Widget *parent_ = nullptr;

    Rect rect_;
    SDL_Texture* texture_ = nullptr;

    bool needRerender_ = true;
    bool isHiden_ = false; 

public:
    Widget(int width, int height, Widget *parent = nullptr);
    virtual ~Widget();


    virtual void renderSelfAction(SDL_Renderer* renderer);
    virtual bool render(SDL_Renderer* renderer);
    virtual bool update();
    virtual bool updateSelfAction();

    // Events: return false to stop propagation (CONSUME), true to continue (PROPAGATE)
    // propagation logic
    virtual bool onMouseDown(const MouseButtonEvent &event);
    virtual bool onMouseUp(const MouseButtonEvent &event);
    virtual bool onMouseWheel(const MouseWheelEvent &event);
    virtual bool onMouseMove(const MouseMotionEvent &event);
    virtual bool onKeyDown(const KeyEvent &event);
    virtual bool onKeyUp(const KeyEvent &event);

    // event self processing logic
    virtual bool onMouseWheelSelfAction(const MouseWheelEvent &event);
    virtual bool onMouseDownSelfAction(const MouseButtonEvent &event);
    virtual bool onMouseUpSelfAction(const MouseButtonEvent &event);
    virtual bool onMouseMoveSelfAction(const MouseMotionEvent &event);
    virtual bool onKeyDownSelfAction(const KeyEvent &event);
    virtual bool onKeyUpSelfAction(const KeyEvent &event);

    // Getters / Setters
    bool isHiden() const { return isHiden_; }
    void hide() { isHiden_ = true; }
    void show() { isHiden_ = false; }
    
    virtual const std::vector<Widget *> &getChildren() const;
    void setPosition(int x, int y) { rect_.x = x; rect_.y = y; }
    void setSize(int w, int h) { rect_.w = w; rect_.h = h; }
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
    Container(int width, int height, Widget *parent=nullptr);
    virtual ~Container();

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
    void addWidget(int x, int y, Widget *widget);
};

class Window : public Container {
protected:
    gm_dot<int, 2> accumulatedRel_ = {0, 0};
    bool replaced_ = true;
public:
    Window(int w, int h, Widget *parent=nullptr) : Container(w, h, parent) {}    

    void renderSelfAction(SDL_Renderer* renderer) override;
    bool onMouseMoveSelfAction(const MouseMotionEvent &event) override;
    bool updateSelfAction() override;
};

// class Button : public Widget {
//     const char *pressedButtonTexturePath_ = nullptr;
//     const char *unpressedButtonTexturePath_ = nullptr;

//     SDL_Texture* pressedButtonTexture_ = nullptr;
//     SDL_Texture* unpressedButtonTexture_ = nullptr;
//     bool textureCreated = false;
   
//     bool pressed_ = false;
//     std::function<void()> onClickFunction_= nullptr;

// public:
//     Button
//     (
//         int w, int h,
//         const char *unpressedButtonTexturePath, const char *pressedButtonTexturePath,
//         std::function<void()> onClickFunction=nullptr, Widget *parent=nullptr
//     );

//     ~Button();
 
// private:    
    

//     void initTexture(SDL_Renderer* renderer);
//     void renderSelfAction(SDL_Renderer* renderer) override;
//     void setPressedTexture(SDL_Renderer* renderer);
//     void setUnPressedTexture(SDL_Renderer* renderer);
//     bool onMouseUpSelfAction(const MouseButtonEvent &event) override;
//     bool onMouseDownSelfAction(const MouseButtonEvent &event) override;
// };

#endif // MyGUI_H
