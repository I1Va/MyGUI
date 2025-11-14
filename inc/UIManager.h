#ifndef UI_MANAGER_H
#define UI_MANAGER_H
#include <vector>
#include <functional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Events.h"
class Widget;


inline constexpr int DEFAULT_FRAME_DELAY_MS = 1000 / 60;

struct UIManagerglobalState {
    Widget *hovered = nullptr;
    Widget *mouseActived = nullptr;
};

class UIManager {
    Widget *wTreeRoot_   = nullptr;
    std::vector<Widget*> modalWidgets_{};
    UIManagerglobalState glState_{};
    Uint32 frameDelayMs_;

    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *mainWindow_ = nullptr;

    std::vector<std::function<void(int)>> userEvents_ = {};

private:
    void globalStateOnMouseWheel(Widget *wgt, const MouseWheelEvent  &event);
    void globalStateOnMouseMove (Widget *wgt, const MouseMotionEvent &event);
    void globalStateOnMouseDown (Widget *wgt, const MouseButtonEvent &event);
    void globalStateOnKeyDown   (Widget *wgt, const KeyEvent         &event);
    void globalStateOnKeyUp     (Widget *wgt, const KeyEvent         &event);

    void handleSDLEvents(bool *running);
    void initWTree(Widget *wgt);

public: // user API
    UIManager(int width, int height, Uint32 frameDelay=DEFAULT_FRAME_DELAY_MS);
    ~UIManager();

    void setMainWidget(int x, int y, Widget *mainWidget);
    void pushModalWidget(int x, int y, Widget *modalWidget);
    void registerHotkey(SDL_KeyCode hotkey, std::function<void()> action);

    void run();
    const Widget *hovered() const { return glState_.hovered; }
    void setHovered(Widget *widget) { glState_.hovered = widget; }
    const Widget *mouseActived() const { return glState_.mouseActived; }
    void setMouseActived(Widget *widget) { glState_.mouseActived = widget; }

    void addUserEvent(std::function<void(int)> userEvent) { userEvents_.push_back(userEvent); };
    TTF_Font* createFont(const char fontPath[], const size_t fontSize);

friend class Widget;
};


#endif // UI_MANAGER_H