#include "Container.h"
#include "Events.h"

Container::Container(int width, int height, Widget *parent)
    : Widget(width, height, parent) {}

Container::~Container() {
    for (Widget *child : children_) delete child;
    children_.clear();
}

bool Container::onMouseDown(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false; 

    for (Widget *child : children_) {
        MouseButtonEvent childLocal = event;

        childLocal.pos.x -= rect_.x;
        childLocal.pos.y -= rect_.y;
    
        if (child->onMouseDown(childLocal)) return true; // stop propagation
    }

    if (onMouseDownSelfAction(event)) return true;

    return false; // propagate
}

bool Container::onMouseUp(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

    for (Widget *child : children_) {
        MouseButtonEvent childLocal = event;
        childLocal.pos.x -= rect_.x;
        childLocal.pos.y -= rect_.y;
    
        if (child->onMouseUp(childLocal)) return true;
    }

    if (onMouseUpSelfAction(event)) return true;

    return false;
}

bool Container::onMouseMove(const MouseMotionEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;
    
    for (Widget *child : children_) {
        MouseMotionEvent childLocal = event;
        childLocal.pos.x -= rect_.x;
        childLocal.pos.y -= rect_.y;

        if (child->onMouseMove(childLocal)) return true;
    }

    if (onMouseMoveSelfAction(event)) return true;
    
    return false;
}

void reorderWidgets(std::vector<std::pair<bool, Widget *>> &reorderingBufer) {
    int left = 0;
    int right = (int)(reorderingBufer.size()) - 1;
    while (right - left > 0) {
        if (!reorderingBufer[left].first && reorderingBufer[right].first) {
            std::swap(reorderingBufer[left], reorderingBufer[right]);
        }
        if (reorderingBufer[left].first) {
            left++;
        }
        if (!reorderingBufer[right].first) {
            right--;
        }
    }
}

bool Container::update() {
    bool updated = updateSelfAction();

    std::vector<std::pair<bool, Widget *>> reorderingBufer(children_.size());

    for (std::size_t i = 0; i < children_.size(); i++) {
        Widget *child = children_[i];
        bool childUpdated = child->update();
        reorderingBufer[i] = {childUpdated, child};
        updated |= childUpdated;
    }

    reorderWidgets(reorderingBufer);
    for (std::size_t i = 0; i < children_.size(); i++) {
        children_[i] = reorderingBufer[i].second;
    }

    if (parent_ && needRerender_) {
        parent_->invalidate();
    }

    return updated;
}

bool Container::render(SDL_Renderer* renderer) {
    assert(renderer);

    if (!needRerender_) return false;

    RendererGuard rendererGuard(renderer);

    if (!texture_) texture_ = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_TARGET,
                                 rect_.w, rect_.h);

    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture_, 255);

    SDL_SetRenderTarget(renderer, texture_);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    renderSelfAction(renderer);
    
    // render children into the texture; children should produce their own texture
    SDL_Rect clip = { 0, 0, rect_.w, rect_.h };
    SDL_RenderSetClipRect(renderer, &clip);
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        Widget *child = *it;
        child->render(renderer);
        if (child->texture()) {
            SDL_Rect chldRect = child->rect();
            SDL_RenderCopy(renderer, child->texture(), NULL, &chldRect);
        }
    }

    needRerender_ = false;  
    return true;
}

const std::vector<Widget *> &Container::getChildren() const {
    return children_;
}

void Container::addWidget(int x, int y, Widget *widget) {
    assert(widget);

    if (widget->parent() == this || widget->parent() == nullptr) {
        setParentImpl(widget, this);
        widget->setPosition(x, y);
        children_.push_back(widget);
    } else {
        std::cerr << "addWidget failed : parent does not match\n";
        return;
    }
}
