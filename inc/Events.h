#ifndef EVENTS_H
#define EVENTS_H

#include <SDL2/SDL.h>


struct Event {
};

struct CordEvent {
    gm_dot<int, 2> pos;
    CordEvent(int x, int y): pos(x, y) {}
    CordEvent() = default;    
};

struct MouseButtonEvent : public CordEvent {
    Uint8 button_;
    MouseButtonEvent(int x, int y, Uint8 button): CordEvent(x, y), button_(button) {}
    MouseButtonEvent() = default;
};

struct MouseMoveEvent : public MouseButtonEvent {
    gm_dot<int, 2> rel;
    MouseMoveEvent(int x, int y,  Uint8 button, int relX, int relY): MouseButtonEvent(x, y, button), rel(relX, relY) {}
    MouseMoveEvent() = default;
};


#endif // EVENTS_H