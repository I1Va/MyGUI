#ifndef MyGUI_H
#define MyGUI_H

#include <vector>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"
#include "Events.h"


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
    Widget *target = nullptr;
};

class UIManager {
    Widget *wTreeRoot_ = nullptr;
    UIManagerglobalState glState_;
    Uint32 frameDelay_;

    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *mainWindow_ = nullptr;

private:
    void handleSDLEvents(bool *running);

public: // user API
    UIManager(int width, int height, Uint32 frameDelay = 32);
    ~UIManager();

    void setMainWidget(Widget *mainWidget);
    void run();

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


    virtual void paintEvent(SDL_Renderer* renderer);
    virtual bool render(SDL_Renderer* renderer);
    virtual void update();

    // Events: return false to stop propagation (CONSUME), true to continue (PROPAGATE)
    virtual bool onMouseDown(const MouseButtonEvent &event);
    virtual bool onMouseUp(const MouseButtonEvent &event);
    virtual bool onMouseMove(const MouseMoveEvent &event);

    // Getters / Setters
    void invalidate() { needRerender_ = true; }
    bool needRerender() const { return needRerender_; }
    Rect rect() const;
    Widget *parent() const;
    void setParent(Widget *parent);
    SDL_Texture* texture();

friend class UIManager;
};

class Container : public Widget {
protected:
    std::vector<Widget *> children_;

    gm_dot<int, 2> accumulatedRel_ = {0, 0};
    bool replaced_ = false;
     
public:
    Container(int x, int y, int width, int height, Widget *parent = nullptr);
    ~Container();

    // Events
    bool onMouseDown(const MouseButtonEvent &event) override;
    bool onMouseUp(const MouseButtonEvent &event) override;
    bool onMouseMove(const MouseMoveEvent &event) override;

    // Stages
    void update() override;
    bool render(SDL_Renderer* renderer) override;

    // User API
    void addWdiget(Widget *widget);
};

// class Window : public Container {
// public:
//     Window(int x, int y, int w, int h);
//     bool onMouseMove(const MouseMoveEvent &event) override;
// };

#endif // MyGUI_H
