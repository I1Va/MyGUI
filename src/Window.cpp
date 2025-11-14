#include "Window.h"
#include "Events.h"
#include "UIManager.h"

bool Window::onMouseMoveSelfAction(const MouseMotionEvent &event) {
    if (this == UIManager_->mouseActived() && event.button == SDL_BUTTON_LEFT) {
        accumulatedRel_ += event.rel;
        replaced_ = true;
        return CONSUME;
    }

    return PROPAGATE;
}

void Window::renderSelfAction(SDL_Renderer* renderer) {
    assert(renderer);

    SDL_Rect widgetRect = {0, 0, rect_.w, rect_.h};
    SDL_SetRenderDrawColor(renderer, DEFAULT_WINDOW_COLOR.r, DEFAULT_WINDOW_COLOR.g, DEFAULT_WINDOW_COLOR.b, DEFAULT_WINDOW_COLOR.a);
    SDL_RenderFillRect(renderer, &widgetRect);
}

bool Window::updateSelfAction() {
    if (replaced_) {
        rect_.x += accumulatedRel_.x;
        rect_.y += accumulatedRel_.y;
        accumulatedRel_ = {0, 0};
        replaced_ = false;
        if (parent_) parent_->invalidate();
        return CONSUME;
    }
    
    return PROPAGATE;
}
