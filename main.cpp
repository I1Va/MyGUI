#include "MGApplication.h"

const int MAIN_WINDOW_WIDTH = 1000;
const int MAIN_WINDOW_HEIGHT = 1000;
const char MAIN_WINDOW_TITLE[] = "MyGUI";

int main(void) {

    MGApplication application;

    try {
        application.setMainWindow(MAIN_WINDOW_TITLE, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    } catch (const std::exception& e) {
        std::cerr << "FATAL : " << e.what() << "\n";
        return -1;
    }

    application.run();

    return 0;
}