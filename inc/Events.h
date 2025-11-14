#ifndef EVENTS_H
#define EVENTS_H
#include <SDL2/SDL.h>

#include "gm_primitives.hpp"


enum EventPropagationNum: bool {
    CONSUME = false,
    PROPAGATE = true
};

struct CordEvent {
    gm_dot<int, 2> pos;
    CordEvent(int x, int y): pos(x, y) {}
    CordEvent() = default;    
};

struct KeyEvent {
    int sym;
    int keymod;

    explicit KeyEvent(int sym, int keymod): sym(sym), keymod(keymod) {}
    KeyEvent() = default;
};

struct MouseWheelEvent {
    gm_dot<int, 2> rot; 

    MouseWheelEvent(int rotX, int rotY): rot(rotX, rotY) {}
    MouseWheelEvent() = default;
};

struct MouseButtonEvent : public CordEvent {
    u_int8_t button;
    MouseButtonEvent(int x, int y, Uint8 button): CordEvent(x, y), button(button) {}
    MouseButtonEvent() = default;
};

struct MouseMotionEvent : public MouseButtonEvent {
    gm_dot<int, 2> rel;
    MouseMotionEvent(int x, int y,  Uint8 button, int relX, int relY): MouseButtonEvent(x, y, button), rel(relX, relY) {}
    MouseMotionEvent() = default;
};


#endif // EVENTS_H