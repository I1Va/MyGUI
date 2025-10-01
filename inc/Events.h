#ifndef EVENTS_H
#define EVENTS_H

#include <SDL2/SDL.h>


class Event {
};

class CordEvent {
    Sint32 x_, y_;
public:
    CordEvent(Sint32 x, Sint32 y): x_(x), y_(y) {}
    CordEvent() = default;    
};

class MouseButtonEvent : public CordEvent {
    Uint8 button_;

public:
    MouseButtonEvent(Sint32 x, Sint32 y, Uint8 button): CordEvent(x, y), button_(button) {}
    MouseButtonEvent() = default;
};


#endif // EVENTS_H