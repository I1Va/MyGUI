#include "Widget.h"
#include "Events.h"

Widget::Widget(int width, int height, Widget *parent)
    : parent_(parent), rect_(0, 0, width, height)
{}

Widget::~Widget() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

void Widget::renderSelfAction(SDL_Renderer* renderer) {
    assert(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
    SDL_Rect full = {0, 0, rect_.w, rect_.h};
    SDL_RenderFillRect(renderer, &full);
}

bool Widget::render(SDL_Renderer* renderer) {
    if (!needRerender_) return false;

    RendererGuard RendererGuard(renderer);

    if (!texture_) texture_ = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_TARGET,
                                 rect_.w, rect_.h);
    assert(texture_);

    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture_, 255);

    SDL_SetRenderTarget(renderer, texture_);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    renderSelfAction(renderer);

    needRerender_ = false;
    return true;
}

bool Widget::updateSelfAction() { return false; }
bool Widget::update() { return updateSelfAction(); }

bool Widget::onMouseWheel(const MouseWheelEvent &event) {
    return onMouseWheelSelfAction(event);
}
bool Widget::onMouseDown(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return PROPAGATE;

    return onMouseDownSelfAction(event);
}
bool Widget::onMouseUp(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return PROPAGATE;

    return onMouseUpSelfAction(event);
}
bool Widget::onMouseMove(const MouseMotionEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return PROPAGATE;

    return onMouseMoveSelfAction(event);
}
bool Widget::onKeyDown(const KeyEvent &event) {
    return onKeyDownSelfAction(event); 
}
bool Widget::onKeyUp(const KeyEvent &event) {
    return onKeyUpSelfAction(event); 
}


bool Widget::onMouseWheelSelfAction(const MouseWheelEvent &event) {
    return PROPAGATE;
}
bool Widget::onMouseDownSelfAction(const MouseButtonEvent &event) {
    return PROPAGATE;
}
bool Widget::onMouseUpSelfAction(const MouseButtonEvent &event) {
    return PROPAGATE;
}
bool Widget::onMouseMoveSelfAction(const MouseMotionEvent &event) {
    return PROPAGATE;
}
bool Widget::onKeyDownSelfAction(const KeyEvent &event) {
    return PROPAGATE;
}
bool Widget::onKeyUpSelfAction(const KeyEvent &event) {
    return PROPAGATE;
}


const std::vector<Widget *> &Widget::getChildren() const {
    static const std::vector<Widget*> empty;
    return empty;
}

Rect Widget::rect() const { return rect_; }
const Widget *Widget::parent() const { return parent_; }
SDL_Texture* Widget::texture() { return texture_; }
