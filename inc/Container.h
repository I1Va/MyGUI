#ifndef CONTAINER_H
#define CONTAINER_H
#include <vector>

#include <SDL2/SDL.h>
#include "Common.h"
#include "Widget.h"

class UIManager;


class Container : public Widget {
protected:
    std::vector<Widget *> children_;
     
public:
    Container(int width, int height, Widget *parent=nullptr);
    virtual ~Container();

    // Events
    bool onMouseDown(const MouseButtonEvent &event) override;
    bool onMouseUp(const MouseButtonEvent &event) override;
    bool onMouseMove(const MouseMotionEvent &event) override;

    // Stages
    bool update() override;
    bool render(SDL_Renderer* renderer) override;

    // Getters / setters
    const std::vector<Widget *> &getChildren() const override;

    // User API
    void addWidget(int x, int y, Widget *widget);
};


#endif // CONTAINER_H