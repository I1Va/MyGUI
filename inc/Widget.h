#ifndef WIDGET_H
#define WIDGET_H
#include <vector>

#include <SDL2/SDL.h>
#include "Common.h"

class UIManager;
class MouseButtonEvent;
class MouseWheelEvent;
class MouseMotionEvent;
class KeyEvent;

class Widget {  
protected:
    UIManager *UIManager_ = nullptr;
    Widget *parent_ = nullptr;

    Rect rect_;
    SDL_Texture* texture_ = nullptr;

    bool needRerender_ = true;
    bool isHiden_ = false; 

    void setParentImpl(Widget* child, Widget* parent) { child->parent_ = parent; }

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
};


#endif // WIDGET_H