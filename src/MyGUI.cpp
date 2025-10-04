#include "MyGUI.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cassert>

// ---------------- RendererGuard ----------------

inline bool isNullRect(const SDL_Rect &rect) {
    return (rect.x == 0 && rect.y == 0 &&
            rect.w == 0 && rect.h == 0);
}

RendererGuard::RendererGuard(SDL_Renderer* renderer) : renderer_(renderer) {
    savedTarget_ = SDL_GetRenderTarget(renderer_);
    SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
    SDL_GetRenderDrawBlendMode(renderer_, &blend_);
    SDL_RenderGetViewport(renderer_, &savedViewport_);
    SDL_RenderGetClipRect(renderer_, &savedClip_);
}

RendererGuard::~RendererGuard() {
    SDL_SetRenderTarget(renderer_, savedTarget_);      
    SDL_SetRenderDrawColor(renderer_, r_, g_, b_, a_); 
    SDL_SetRenderDrawBlendMode(renderer_, blend_);     

    if (!isNullRect(savedViewport_))
        SDL_RenderSetViewport(renderer_, &savedViewport_);
    else
        SDL_RenderSetViewport(renderer_, nullptr);

    if (!isNullRect(savedClip_))
        SDL_RenderSetClipRect(renderer_, &savedClip_);
    else
        SDL_RenderSetClipRect(renderer_, nullptr);
}


// ---------------- Rect / util ----------------

Rect::Rect(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

gm_dot<int, 2> Rect::pos() const {
    return { x, y };
}

bool isInsideRect(const Rect& rect, const int x, const int y) {
    return (x >= rect.x) && (x < rect.x + rect.w) && (y >= rect.y) && (y < rect.y + rect.h);
}

// ---------------- UIManager ----------------

UIManager::UIManager(int width, int height, Uint32 frameDelay)
    : frameDelay_(frameDelay)
{
    mainWindow_ = SDL_CreateWindow(
        nullptr,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        0);

    if (!mainWindow_) throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

    renderer_ = SDL_CreateRenderer(mainWindow_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        SDL_DestroyWindow(mainWindow_);
        mainWindow_ = nullptr;
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }
}

UIManager::~UIManager() {
    if (wTreeRoot_) delete wTreeRoot_;
    if (mainWindow_) SDL_DestroyWindow(mainWindow_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    SDL_Quit();
}

void UIManager::setMainWidget(Widget *mainWidget) {
    if (wTreeRoot_ != nullptr) {
        std::cerr << "setMainWidget failed : wTreeRoot != nullptr\n";
        return;
    }

    wTreeRoot_ = mainWidget;
}

void UIManager::handleSDLEvents(bool *running) {
    static gm_dot<int, 2> prevMousePos = {0, 0};
    gm_dot<int, 2> curMousePos = {0, 0};

    SDL_Event SDLEvent;

    while (SDL_PollEvent(&SDLEvent)) {
        if (SDLEvent.type == SDL_QUIT || SDLEvent.type == SDL_KEYDOWN && SDLEvent.key.keysym.sym == SDLK_ESCAPE) {
            *running = false;
            break;
        }

        switch (SDLEvent.type) {
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&curMousePos.x, &curMousePos.y);
                wTreeRoot_->onMouseMove({curMousePos.x, curMousePos.y, SDLEvent.button.button, 
                                        curMousePos.x - prevMousePos.x, curMousePos.y - prevMousePos.y});
                break;
            case SDL_MOUSEBUTTONDOWN:
                wTreeRoot_->onMouseDown({SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button});
                break;
            case SDL_MOUSEBUTTONUP:
                wTreeRoot_->onMouseUp({SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button});
                break;
            
            default:
                break;
        }
        SDL_GetMouseState(&prevMousePos.x, &prevMousePos.y);
    }
}

void UIManager::run() {
    if (!renderer_ || !mainWindow_) {
        throw std::runtime_error("UIManager::run: window/renderer not initialized");
    }

    bool running = true;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        handleSDLEvents(&running);

        // update tree
        if (wTreeRoot_) wTreeRoot_->update();

        // render pass
        SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
        SDL_RenderClear(renderer_);

        if (wTreeRoot_) {
            wTreeRoot_->render(renderer_);
            assert(wTreeRoot_->texture());
            SDL_Rect dst = wTreeRoot_->rect();
            SDL_RenderCopy(renderer_, wTreeRoot_->texture(), NULL, &dst);
        }

        SDL_RenderPresent(renderer_);

        // frame pacing
        // Uint32 frameTime = SDL_GetTicks() - frameStart;
        // if (frameDelay_ > frameTime) SDL_Delay(frameDelay_ - frameTime);
    }
}

// ---------------- Widget ----------------

Widget::Widget(int x, int y, int width, int height, Widget *parent)
    : parent_(parent), rect_(x, y, width, height)
{}

Widget::~Widget() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

void Widget::paintEvent(SDL_Renderer* renderer) {
    assert(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
    SDL_Rect full = {0, 0, rect_.w, rect_.h};
    SDL_RenderFillRect(renderer, &full);
}

bool Widget::render(SDL_Renderer* renderer) {
    if (!needRerender_) return false;

    RendererGuard RendererGuard(renderer);

    if (texture_) SDL_DestroyTexture(texture_);        

    texture_ = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_TARGET,
                                 rect_.w, rect_.h);


    SDL_SetRenderTarget(renderer, texture_);
    SDL_RenderClear(renderer);

    paintEvent(renderer);

    needRerender_ = false;

    return true;
}

void Widget::update() {}

bool Widget::onMouseDown(const MouseButtonEvent &event) {
    return true;
}
bool Widget::onMouseUp(const MouseButtonEvent &event) {
    return true;
}
bool Widget::onMouseMove(const MouseMoveEvent &event) {
    return true;
}

Rect Widget::rect() const { return rect_; }
Widget *Widget::parent() const { return parent_; }
void Widget::setParent(Widget *parent) { parent_ = parent; }
SDL_Texture* Widget::texture() { return texture_; }

// ---------------- Container ----------------

Container::Container(int x, int y, int width, int height, Widget *parent)
    : Widget(x, y, width, height, parent) {}

Container::~Container() {
    for (Widget *child : children_) delete child;
    children_.clear();
}

bool Container::onMouseDown(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return true; 

    for (Widget *child : children_) {
        MouseButtonEvent childLocal = event;

        childLocal.pos.x -= child->rect().x;
        childLocal.pos.y -= child->rect().y;
        if (!child->onMouseDown(childLocal)) {
            return false; // stop propagation
        }
    }

    return true; // propagate
}

bool Container::onMouseUp(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return true;

    for (Widget *child : children_) {
        MouseButtonEvent childLocal = event;
        childLocal.pos.x -= child->rect().x;
        childLocal.pos.y -= child->rect().y;
        if (!child->onMouseUp(childLocal)) {
            return false;
        }
    }

    return true;
}

bool Container::onMouseMove(const MouseMoveEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) {
        return true;
    }

    for (Widget *child : children_) {
        MouseMoveEvent childLocal = event;
        childLocal.pos.x -= rect_.x;
        childLocal.pos.y -= rect_.y;
        if (!child->onMouseMove(childLocal)) {
            return false;
        }
    }

    
    if (event.button_ == SDL_BUTTON_LEFT) {
        accumulatedRel_ += event.rel;
        replaced_ = true;
        return false;
    }
    
    return true;
}

void Container::update() {
    if (replaced_) {
        rect_.x += accumulatedRel_.x;
        rect_.y += accumulatedRel_.y;
        accumulatedRel_ = {0, 0};
        replaced_ = false;
        if (parent_) {
            parent_->invalidate();
        }
    }

    for (Widget *child : children_) child->update();

    if (parent_ && needRerender_) {
        parent_->invalidate();
    }
}

bool Container::render(SDL_Renderer* renderer) {
    assert(renderer);

    bool needsReRender = needRerender_;

    for (Widget *child : children_) {
        needsReRender |= child->needRerender();
    }
    if (!needsReRender) return false;

    RendererGuard rendererGuard(renderer);

    if (texture_) SDL_DestroyTexture(texture_);     

    texture_ = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_TARGET,
                                 rect_.w, rect_.h);

    SDL_SetRenderTarget(renderer, texture_);
    SDL_RenderClear(renderer);
    paintEvent(renderer);
    
    SDL_Rect clip = { 0, 0, rect_.w, rect_.h };
    SDL_RenderSetClipRect(renderer, &clip);
    // render children into the texture; children should produce their own texture
    for (Widget *child : children_) {
        child->render(renderer);
        if (child->texture()) {
            SDL_Rect chldRect = child->rect();
            SDL_RenderCopy(renderer, child->texture(), NULL, &chldRect);
        }
    }

    needRerender_ = false;  

    return true;
}

void Container::addWdiget(Widget *widget) {
    if (widget->parent() == this || widget->parent() == nullptr) {
        widget->setParent(this);
        children_.push_back(widget);
    } else {
        std::cerr << "addWdiget failed : parent does not match\n";
    }
}

// ---------------- Window ----------------


// bool isMouseEvent(const SDL_Event &event) {
//     return  event.type == SDL_MOUSEMOTION ||
//             event.type == SDL_MOUSEBUTTONDOWN || 
//             event.type == SDL_MOUSEBUTTONUP || 
//             event.type == SDL_MOUSEWHEEL;
// }


// SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer) {
//     assert(renderer);

//     if (!texturePath) return nullptr;

//     SDL_Texture* resultTexture = nullptr;
//     SDL_Surface* surface = IMG_Load(texturePath);
//     if (!surface) std::cerr << "MGButton texture load failed : " << texturePath << "\n";

//     if (surface) {
//         resultTexture = SDL_CreateTextureFromSurface(renderer, surface);
//         SDL_FreeSurface(surface);
//     }

//     return resultTexture; 
// }

