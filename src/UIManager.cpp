#include <cassert>
#include <iostream>

#include "UIManager.h"
#include "Widget.h"

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

    renderer_ = SDL_CreateRenderer(mainWindow_, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
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


bool UIManager::modalWidgetsOnMouseMove(const MouseMotionEvent &event) {
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onMouseMove(event) == CONSUME) return CONSUME;
    }

    return PROPAGATE;
}

bool UIManager::modalWidgetsOnKeyDown(const KeyEvent &event) {    
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onKeyDown(event) == CONSUME) return CONSUME;
    }

    return PROPAGATE;
}

bool UIManager::modalWidgetsOnKeyUp(const KeyEvent &event) {    
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onKeyUp(event) == CONSUME) return CONSUME;
    }

    return PROPAGATE;
}

bool UIManager::modalWidgetsOnMouseWheel(const MouseWheelEvent &event) {    
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onMouseWheel(event) == CONSUME) return CONSUME;
    }

    return PROPAGATE;
}

bool UIManager::modalWidgetsOnMouseDown(const MouseButtonEvent &event) {
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onMouseDown(event) == CONSUME) return CONSUME;
    }
    return PROPAGATE;
}

bool UIManager::modalWidgetsOnMouseUp(const MouseButtonEvent &event) {
    for (Widget *modalWgt : modalWidgets_) {
        if (modalWgt->isHiden()) continue;
        if (modalWgt->onMouseUp(event) == CONSUME) return CONSUME;
    }
    return PROPAGATE;
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
                
                if (modalWidgetsOnKeyDown(keyEvent) == CONSUME) break;
                if (wTreeRoot_) globalStateOnKeyDown(wTreeRoot_, keyEvent);
                if (glState_.mouseActived) glState_.mouseActived->onKeyDown(keyEvent);
                break;
            
            case SDL_KEYUP:
                keyEvent = KeyEvent(SDLEvent.key.keysym.sym, SDLEvent.key.keysym.mod);

                if (modalWidgetsOnKeyUp(keyEvent) == CONSUME) break;
                if (wTreeRoot_) globalStateOnKeyDown(wTreeRoot_, keyEvent);
                if (glState_.mouseActived) glState_.mouseActived->onKeyUp(keyEvent);
                break;

            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&curMousePos.x, &curMousePos.y);
                mouseMotionEvent = MouseMotionEvent(curMousePos.x, curMousePos.y, SDLEvent.button.button, 
                                    curMousePos.x - prevMousePos.x, curMousePos.y - prevMousePos.y);
                
                if (modalWidgetsOnMouseMove(mouseMotionEvent) == CONSUME) break;
                if (wTreeRoot_) globalStateOnMouseMove(wTreeRoot_, mouseMotionEvent);
                if (wTreeRoot_) wTreeRoot_->onMouseMove(mouseMotionEvent);
                break;
            
            case SDL_MOUSEWHEEL:
                mouseWheelEvent = MouseWheelEvent(SDLEvent.wheel.x, SDLEvent.wheel.y);
                    
                if (modalWidgetsOnMouseWheel(mouseWheelEvent) == CONSUME) break;
                if (wTreeRoot_) globalStateOnMouseWheel(wTreeRoot_, mouseWheelEvent);
                if (glState_.mouseActived) glState_.mouseActived->onMouseWheel(mouseWheelEvent);
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                mouseButtonEvent = MouseButtonEvent(SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button);
                
                if (modalWidgetsOnMouseDown(mouseButtonEvent) == CONSUME) break;
                if (wTreeRoot_) globalStateOnMouseDown(wTreeRoot_, mouseButtonEvent);
                if (wTreeRoot_) wTreeRoot_->onMouseDown(mouseButtonEvent);
                break;
    
            case SDL_MOUSEBUTTONUP:
                mouseButtonEvent = MouseButtonEvent(SDLEvent.button.x, SDLEvent.button.y, SDLEvent.button.button);

                if (modalWidgetsOnMouseUp(mouseButtonEvent) == CONSUME) break;
                if (wTreeRoot_) wTreeRoot_->onMouseUp(mouseButtonEvent);
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

void UIManager::updatePass() {
    if (wTreeRoot_) wTreeRoot_->update();

    for (Widget *modalWgt : modalWidgets_) {
        modalWgt->update();
    }

    for (std::function<void(int)> userEvent : userEvents_)
        userEvent(frameDelayMs_);
}

void UIManager::renderPass() {
    if (wTreeRoot_) {
        wTreeRoot_->render(renderer_);
        SDL_Rect dst = wTreeRoot_->rect();
        SDL_RenderCopy(renderer_, wTreeRoot_->texture(), NULL, &dst);
    }

    for (Widget *modalWgt : modalWidgets_)  {
        if (modalWgt->isHiden()) continue;

        modalWgt->render(renderer_);
        assert(modalWgt->texture());
    
        SDL_Rect dst = modalWgt->rect();
        SDL_RenderCopy(renderer_, modalWgt->texture(), NULL, &dst);
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
        updatePass();
        
        // render 
        SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
        SDL_RenderClear(renderer_);
        renderPass();
        SDL_RenderPresent(renderer_);

        // frame pacing
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelayMs_ > frameTime) SDL_Delay(frameDelayMs_ - frameTime);
    }
}