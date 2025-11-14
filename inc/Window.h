#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include "Container.h"
#include "Common.h"

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

#endif // WINDOW_H
