#include "MyGUI.h"

const int MAIN_WINDOW_WIDTH = 800;
const int MAIN_WINDOW_HEIGHT = 600;
const char MAIN_WINDOW_TITLE[] = "MyGUI";

const char PRESSED_BUTTON_IMAGE_PATH[] = "images/pressedButton.png";
const char UNPRESSED_BUTTON_IMAGE_PATH[] = "images/unpressedButton.png";

void ReactorUpdate() {
    std::cout << "Isolate Reactor has updated!\n";
}

void ReactorAddMolecule() {
    std::cout << "ReactorAddMolecule!\n";
}

int main(void) {

    MGApplication application;

    try {
        application.setMainWindow(MAIN_WINDOW_TITLE, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    } catch (const std::exception& e) {
        std::cerr << "FATAL : " << e.what() << "\n";
        return -1;
    }

    SignalManager *signalManager = application.getSignalManager();
    MGMainWindow *mainWindow = application.getMainWindow();
    MGWindow *window = mainWindow->addWindow(200, 200, 400, 400);
    MGCanvas *canvas = window->addCanvas(30, 30, 1000, 1000);
    window->addCanvas(60, 60, 300, 300);
    MGButton *button = window->addButton(30, 30, 100, 100, PRESSED_BUTTON_IMAGE_PATH, UNPRESSED_BUTTON_IMAGE_PATH, ReactorAddMolecule);

    application.addEventToMainLoop(ReactorUpdate);

    application.run();
    return 0;
}