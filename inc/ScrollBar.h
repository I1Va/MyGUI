#ifndef SCROLL_BAR_H
#define SCROLL_BAR_H

#include "gm_primitives.hpp"
#include "MyGUI.h"

class ThumbButton : public Button {
    gm_dot<int, 2> accumulatedRel_ = {};

    bool replaced_ = false;

    SDL_Rect movingArea_ = {};

public:
    ThumbButton
    (
        int width, int height,
        SDL_Rect movingArea,
        const ButtonTexturePath texturePath,
        std::function<void()> onClickFunction=nullptr, Widget *parent=nullptr
    ): Button(width, height, texturePath.unpressed, texturePath.pressed, onClickFunction, parent),
       movingArea_(movingArea) {};

    bool onMouseMoveSelfAction(const MouseMotionEvent &event) {
        if (this == UIManager_->mouseActived() && event.button == SDL_BUTTON_LEFT) {
            accumulatedRel_ += event.rel;
            replaced_ = true;
            return true;
        }

        return false;
    }

    void clampPos() {
        rect_.x = std::clamp(rect_.x, movingArea_.x, movingArea_.x + movingArea_.w);
        rect_.y = std::clamp(rect_.y, movingArea_.y, movingArea_.y + movingArea_.h);
    }

    bool updateSelfAction() {
        if (replaced_) {
            rect_.x += accumulatedRel_.x;
            rect_.y += accumulatedRel_.y;

            clampPos();
            
            accumulatedRel_ = {0, 0};
            replaced_ = false;
            if (parent_) parent_->invalidate();
            return true;
        }
        
        return false;
    }
};

class ScrollBar : public Container {
    static constexpr double BUTTON_LAYOUT_SHARE_ = 0.1; 
    static constexpr double THUMB_MOVING_DELTA = 0.05;

    std::function<void(double)> onScroll_;

    gm_dot<int, 2> startThumbPos_;
    double percentage_ = 0; 
    bool isHorizontal_;

    gm_dot<int, 2> buttonSize_ = {};

    SDL_Rect thumbMovingArea_ = {};

    Button *bottomButton_   = nullptr;
    Button *topButton_      = nullptr;
    ThumbButton *thumbButton_    = nullptr;

private:
    gm_dot<int, 2> getThumbPos(double percentage) {
        percentage = std::clamp(percentage, 0.0, 100.0);
    
        double len = (isHorizontal_? rect_.w - 3 * buttonSize_.x : rect_.h - 3 * buttonSize_.y);
        double x = (isHorizontal_? buttonSize_.x + percentage * len : 0);
        double y = (!isHorizontal_? buttonSize_.y + percentage * len : 0);
        return {(int) x, (int) y};
    }

    void move(double deltaPercent) {
        const double newPercentage = std::clamp(percentage_ + deltaPercent, 0.0, 1.0);
        const double realDelta = newPercentage - percentage_;

        if (onScroll_) onScroll_(newPercentage);

        gm_dot<int, 2> thumbPos = getThumbPos(percentage_);
        
        int relX = (isHorizontal_ ? realDelta : 0) * (rect_.w - 3 * buttonSize_.x);
        int relY = (!isHorizontal_ ? realDelta : 0) * (rect_.h - 3 * buttonSize_.y);

        thumbButton_->setPosition(thumbPos.x + relX, thumbPos.y + relY);
        thumbButton_->clampPos();

        percentage_ = newPercentage;
    }

    double getPercentFromRelativePos(int x, int y) {
        if (isHorizontal_) return ((double) x) / (rect_.w - 3 * buttonSize_.x);
        return ((double) y) / (rect_.h - 3 * buttonSize_.y);
    }



public:
    ScrollBar
    (
        int width, int height, std::function<void(double)> onScroll, 
        const ButtonTexturePath &scrollBarTopBtnPath,
        const ButtonTexturePath &scrollBarBottomBtnPath,
        const ButtonTexturePath &scrollThumbBtnPath,
        bool isHorizontal = true,
        Widget *parent=nullptr
    ): 
        Container(width, height, parent), isHorizontal_(isHorizontal), onScroll_(onScroll)
    {
        buttonSize_ = 
        {
            (int) (( isHorizontal ? BUTTON_LAYOUT_SHARE_ : 1) * rect_.w),
            (int) ((!isHorizontal ? BUTTON_LAYOUT_SHARE_ : 1) * rect_.h)
        };

        topButton_ = new Button(buttonSize_.x, buttonSize_.y, scrollBarTopBtnPath.unpressed, scrollBarTopBtnPath.pressed, 
            [this] { move(THUMB_MOVING_DELTA); }, this);
        addWidget(( isHorizontal ? rect_.w - buttonSize_.x : 0), (!isHorizontal ? rect_.h - buttonSize_.y : 0), topButton_);

        bottomButton_ = new Button(buttonSize_.x, buttonSize_.y, scrollBarBottomBtnPath.unpressed, scrollBarBottomBtnPath.pressed, 
            [this] { move(-THUMB_MOVING_DELTA); }, this);
        addWidget(0, 0, bottomButton_);

        thumbMovingArea_.x = (isHorizontal ? buttonSize_.x : 0);
        thumbMovingArea_.y = (!isHorizontal ? buttonSize_.y : 0);
        thumbMovingArea_.w = (isHorizontal ? rect_.w - 3 * buttonSize_.x : 0);
        thumbMovingArea_.h = (!isHorizontal ? rect_.h - 3 * buttonSize_.y : 0);

        startThumbPos_ = getThumbPos(percentage_);
        
        thumbButton_ = new ThumbButton(buttonSize_.x, buttonSize_.y, thumbMovingArea_, scrollThumbBtnPath, nullptr, this);
        addWidget(startThumbPos_.x, startThumbPos_.y, thumbButton_);
    }

    bool updateSelfAction() override {
        bool updated = false;

        int dx = thumbButton_->rect().x - startThumbPos_.x;
        int dy = thumbButton_->rect().y - startThumbPos_.y;
        double newPercentage = getPercentFromRelativePos(dx, dy);

        if (newPercentage != percentage_) {
            percentage_ = newPercentage;
            setRerenderFlag();
            if (onScroll_) onScroll_(percentage_);
            updated = true;
        }
    
        return updated;
    }

    void renderSelfAction(SDL_Renderer* renderer) override {
        assert(renderer);

        SDL_Rect widgetRect = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, DEFAULT_WINDOW_COLOR.r, DEFAULT_WINDOW_COLOR.g, DEFAULT_WINDOW_COLOR.b, DEFAULT_WINDOW_COLOR.a);
        SDL_RenderFillRect(renderer, &widgetRect);
    }

    bool onMouseDownSelfAction(const MouseButtonEvent &event) override {
        std::cout << "Scroll Bar :  hitting a free area TODO!\n";
        return true;
    }
};


#endif // SCROLL_BAR_H