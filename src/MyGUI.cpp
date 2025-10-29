#include "MyGUI.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cassert>

// ---------------- Textures ---------------------

SDL_Texture* createTexture(const char *texturePath, SDL_Renderer* renderer) {
    assert(renderer);

    if (!texturePath) return nullptr;

    SDL_Texture* resultTexture = nullptr;
    SDL_Surface* surface = IMG_Load(texturePath);
    if (!surface) std::cerr << "Texture load failed : " << texturePath << "\n";

    if (surface) {
        resultTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    return resultTexture;
}

Uint32 SDL2gfxColorToUint32(SDL_Color c) {
    return (static_cast<Uint32>(c.a) << 24) |  
           (static_cast<Uint32>(c.b) << 16) |  
           (static_cast<Uint32>(c.g) << 8)  | 
            static_cast<Uint32>(c.r);          
}

SDL_Color Uint32ToSDL2gfxColor(Uint32 value) {
    SDL_Color color;
    color.r = value & 0xFF;
    color.g = (value >> 8) & 0xFF;
    color.b = (value >> 16) & 0xFF;
    color.a = (value >> 24) & 0xFF;
    return color;
}

SDL_Texture* createFontTexture(TTF_Font* font, const char text[], const int textSize, SDL_Color textColor, SDL_Renderer* renderer) {
    assert(font);
    assert(renderer);
    assert(text);


    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    assert(textSurface);

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    assert(textTexture);

    SDL_FreeSurface(textSurface);

    return textTexture;
}

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
    if (renderer_)
        SDL_SetRenderTarget(renderer_, savedTarget_); // restore previous render target
            
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

inline bool isInsideRect(const Rect& rect, const int x, const int y) {
    return (x >= rect.x) && (x < rect.x + rect.w) && (y >= rect.y) && (y < rect.y + rect.h);
}

inline bool isMouseSDLevent(const SDL_Event &event) {
    return 
        event.type == SDL_MOUSEMOTION       || 
        event.type == SDL_MOUSEBUTTONDOWN   ||
        event.type == SDL_MOUSEBUTTONUP     ;
}

// ---------------- UIManager ----------------

UIManager::UIManager(int width, int height, Uint32 frameDelay)
    : frameDelayMs_(frameDelay)
{
    mainWindow_ = SDL_CreateWindow(
        nullptr,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        0);

    assert(mainWindow_);

    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init: %s", TTF_GetError());
        assert(0);
    }

    renderer_ = SDL_CreateRenderer(mainWindow_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(renderer_);
}

UIManager::~UIManager() {
    if (wTreeRoot_) delete wTreeRoot_;
    if (mainWindow_) SDL_DestroyWindow(mainWindow_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    SDL_Quit();
}

TTF_Font* UIManager::createFont(const char fontPath[], const size_t fontSize) {
    assert(fontPath);

    TTF_Font* font_ = TTF_OpenFont(fontPath, fontSize); 
    if (!font_) {
        SDL_Log("TTF_OpenFont: %s", TTF_GetError());
        assert(0);
    }

    return font_;
}

void UIManager::setMainWidget(int x, int y, Widget *mainWidget) {
    if (wTreeRoot_ != nullptr) {
        std::cerr << "setMainWidget failed : wTreeRoot != nullptr\n";
        return;
    }
    mainWidget->setPosition(x, y);
    wTreeRoot_ = mainWidget;
}

void UIManager::globalStateOnMouseMove(Widget *wgt, const MouseMotionEvent &event) {
    if (!isInsideRect(wgt->rect(), event.pos.x, event.pos.y)) return;
    
    if (event.button == SDL_BUTTON_LEFT) glState_.hovered = wgt;
    
    for (auto it = wgt->getChildren().rbegin(); it != wgt->getChildren().rend(); ++it) {
        MouseMotionEvent childLocal = event;
        childLocal.pos.x -= wgt->rect().x;
        childLocal.pos.y -= wgt->rect().y;

        globalStateOnMouseMove(*it, childLocal);
    }
}

void UIManager::globalStateOnKeyDown(Widget *wgt, const KeyEvent &event) {    
    return;
}

void UIManager::globalStateOnKeyUp(Widget *wgt, const KeyEvent &event) {    
    return;
}

void UIManager::globalStateOnMouseWheel(Widget *wgt, const MouseWheelEvent &event) {    
    return;
}

void UIManager::globalStateOnMouseDown(Widget *wgt, const MouseButtonEvent &event) {
    if (!isInsideRect(wgt->rect(), event.pos.x, event.pos.y)) return;
    
    if (event.button == SDL_BUTTON_LEFT) glState_.mouseActived = wgt;
    
    for (auto it = wgt->getChildren().rbegin(); it != wgt->getChildren().rend(); ++it) {
        MouseButtonEvent childLocal = event;
        childLocal.pos.x -= wgt->rect().x;
        childLocal.pos.y -= wgt->rect().y;

        globalStateOnMouseDown(*it, childLocal);
    }
}

void UIManager::handleSDLEvents(bool *running) {
   
    static gm_dot<int, 2> prevMousePos = {0, 0};
    gm_dot<int, 2> curMousePos = {0, 0};

    SDL_Event SDLEvent = {};
    MouseButtonEvent mouseButtonEvent = {};
    MouseMotionEvent mouseMotionEvent = {};
    MouseWheelEvent  mouseWheelEvent  = {};
    KeyEvent         keyEvent         = {};
    
    while (SDL_PollEvent(&SDLEvent)) {
        if (SDLEvent.type == SDL_QUIT || SDLEvent.type == SDL_KEYDOWN && SDLEvent.key.keysym.sym == SDLK_ESCAPE) {
            *running = false;
            break;
        }

       
        switch (SDLEvent.type) {
            case SDL_KEYDOWN:
                keyEvent = KeyEvent(SDLEvent.key.keysym.sym, SDLEvent.key.keysym.mod);
                globalStateOnKeyDown(wTreeRoot_, keyEvent);
                if (glState_.mouseActived) glState_.mouseActived->onKeyDown(keyEvent);
                break;
            
            case SDL_KEYUP:
                keyEvent = KeyEvent(SDLEvent.key.keysym.sym, SDLEvent.key.keysym.mod);
                globalStateOnKeyDown(wTreeRoot_, keyEvent);
                if (glState_.mouseActived) glState_.mouseActived->onKeyUp(keyEvent);
                break;

            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&curMousePos.x, &curMousePos.y);
                mouseMotionEvent = MouseMotionEvent(curMousePos.x, curMousePos.y, SDLEvent.button.button, 
                                    curMousePos.x - prevMousePos.x, curMousePos.y - prevMousePos.y);

                globalStateOnMouseMove(wTreeRoot_, mouseMotionEvent);
                wTreeRoot_->onMouseMove(mouseMotionEvent);
                break;
            
            case SDL_MOUSEWHEEL:
                mouseWheelEvent = MouseWheelEvent(SDLEvent.wheel.x, SDLEvent.wheel.y);
            
                globalStateOnMouseWheel(wTreeRoot_, mouseWheelEvent);
                if (glState_.mouseActived) glState_.mouseActived->onMouseWheel(mouseWheelEvent);
                
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                mouseButtonEvent = MouseButtonEvent(SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button);
                    
                globalStateOnMouseDown(wTreeRoot_, mouseButtonEvent);
                wTreeRoot_->onMouseDown(mouseButtonEvent);
                break;
    
            case SDL_MOUSEBUTTONUP:
                mouseButtonEvent = MouseButtonEvent(SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button);

                wTreeRoot_->onMouseUp(mouseButtonEvent);
                break;
            
            default:
                break;
        }
        SDL_GetMouseState(&prevMousePos.x, &prevMousePos.y);
    }
}

void UIManager::initWTree(Widget *wgt) {
    assert(wgt);

    wgt->UIManager_ = this;
    for (Widget * child : wgt->getChildren()) {
        initWTree(child);
    }
}

void UIManager::run() {
    static bool firstRun = true;
    if (firstRun && wTreeRoot_) initWTree(wTreeRoot_);
    firstRun = false;

    if (!renderer_ || !mainWindow_)
        throw std::runtime_error("UIManager::run: window/renderer not initialized");

    bool running = true;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        handleSDLEvents(&running);

        // updates
        if (wTreeRoot_) wTreeRoot_->update();
        for (std::function<void(int)> userEvent : userEvents_)
            userEvent(frameDelayMs_);

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
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelayMs_ > frameTime) SDL_Delay(frameDelayMs_ - frameTime);
    }
}

// ---------------- Widget ----------------

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

    SDL_SetRenderTarget(renderer, texture_);
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
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

    return onMouseDownSelfAction(event);
}
bool Widget::onMouseUp(const MouseButtonEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

    return onMouseUpSelfAction(event);
}
bool Widget::onMouseMove(const MouseMotionEvent &event) {
    if (!isInsideRect(rect_, event.pos.x, event.pos.y)) return false;

    return onMouseMoveSelfAction(event);
}
bool Widget::onKeyDown(const KeyEvent &event) {
    return onKeyDownSelfAction(event); 
}
bool Widget::onKeyUp(const KeyEvent &event) {
    return onKeyUpSelfAction(event); 
}


bool Widget::onMouseWheelSelfAction(const MouseWheelEvent &event) {
    return true;
}
bool Widget::onMouseDownSelfAction(const MouseButtonEvent &event) {
    return true;
}
bool Widget::onMouseUpSelfAction(const MouseButtonEvent &event) {
    return true;
}
bool Widget::onMouseMoveSelfAction(const MouseMotionEvent &event) {
    return true;
}
bool Widget::onKeyDownSelfAction(const KeyEvent &event) {
    return true;
}
bool Widget::onKeyUpSelfAction(const KeyEvent &event) {
    return true;
}


const std::vector<Widget *> &Widget::getChildren() const {
    static const std::vector<Widget*> empty;
    return empty;
}

Rect Widget::rect() const { return rect_; }
const Widget *Widget::parent() const { return parent_; }
SDL_Texture* Widget::texture() { return texture_; }

// ---------------- Container ----------------

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

    SDL_SetRenderTarget(renderer, texture_);
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
        widget->parent_ = this;
        widget->setPosition(x, y);
        children_.push_back(widget);
    } else {
        std::cerr << "addWidget failed : parent does not match\n";
        return;
    }
}

// ---------------- Window ----------------

bool Window::onMouseMoveSelfAction(const MouseMotionEvent &event) {
    if (this == UIManager_->mouseActived() && event.button == SDL_BUTTON_LEFT) {
        accumulatedRel_ += event.rel;
        replaced_ = true;
        return true;
    }

    return false;
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
        return true;
    }
    
    return false;
}

// ---------------- Button ----------------

Button::Button
(
    int w, int h,
    const char *unpressedButtonTexturePath, const char *pressedButtonTexturePath,
    std::function<void()> onClickFunction, Widget *parent
): 
    Widget(w, h, parent),  
    pressedButtonTexturePath_(pressedButtonTexturePath),
    unpressedButtonTexturePath_(unpressedButtonTexturePath),
    onClickFunction_(onClickFunction)
{}

Button::~Button() {
    if (pressedButtonTexture_) SDL_DestroyTexture(pressedButtonTexture_);
    if (unpressedButtonTexture_) SDL_DestroyTexture(unpressedButtonTexture_);
}

void Button::initTexture(SDL_Renderer* renderer) {
    assert(renderer);

    if (textureCreated) return;
    textureCreated = true;

    pressedButtonTexture_ = createTexture(pressedButtonTexturePath_, renderer);    
    unpressedButtonTexture_ = createTexture(unpressedButtonTexturePath_, renderer);    
}

void Button::renderSelfAction(SDL_Renderer* renderer) {
    initTexture(renderer);

    if (pressed_) {
        setPressedTexture(renderer);
    } else {
        setUnPressedTexture(renderer);
    }
}

void Button::setPressedTexture(SDL_Renderer* renderer) {
    assert(renderer);

    SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
    SDL_RenderCopy(renderer, pressedButtonTexture_, NULL, &buttonRect);
}

void Button::setUnPressedTexture(SDL_Renderer* renderer) {
    assert(renderer);

    SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
    SDL_RenderCopy(renderer, unpressedButtonTexture_, NULL, &buttonRect);  
}

bool Button::onMouseUpSelfAction(const MouseButtonEvent &event) {
    if (event.button == SDL_BUTTON_LEFT) {
        pressed_ = false;
        setRerenderFlag();
        return true;
    }
    return false;
}

bool Button::onMouseDownSelfAction(const MouseButtonEvent &event) {
    if (event.button == SDL_BUTTON_LEFT) {
        pressed_ = true;
        setRerenderFlag();
        if (onClickFunction_) onClickFunction_();
        return true;
    }
    return false;
}

