#include "MGApplication.h"

const int MAIN_WINDOW_WIDTH = 800;
const int MAIN_WINDOW_HEIGHT = 600;
const char MAIN_WINDOW_TITLE[] = "MyGUI";

const char PRESSED_BUTTON_IMAGE_PATH[] = "images/pressedButton.png";
const char UNPRESSED_BUTTON_IMAGE_PATH[] = "images/unpressedButton.png";

int main(void) {

    MGApplication application;

    try {
        application.setMainWindow(MAIN_WINDOW_TITLE, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    } catch (const std::exception& e) {
        std::cerr << "FATAL : " << e.what() << "\n";
        return -1;
    }

    MGWindow *window1 = application.mainWindow()->addWindow(0, 0, 255, 255);
    MGWindow *window2 = application.mainWindow()->addWindow(300, 300, 400, 400);
    window1->addCanvas(30, 30, 1000, 1000);
    window2->addCanvas(60, 60, 300, 300);
    window2->addButton(30, 30, 100, 100, PRESSED_BUTTON_IMAGE_PATH, UNPRESSED_BUTTON_IMAGE_PATH);



    application.run();


    return 0;
}