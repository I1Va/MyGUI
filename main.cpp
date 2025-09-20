#include "MyGUI.h"

const int MAIN_WINDOW_WIDTH = 800;
const int MAIN_WINDOW_HEIGHT = 600;
const char MAIN_WINDOW_TITLE[] = "MyGUI";

const char PRESSED_BUTTON_IMAGE_PATH[] = "images/pressedButton.png";
const char UNPRESSED_BUTTON_IMAGE_PATH[] = "images/unpressedButton.png";

void RedrawCanvas() {
    bool redraw = true;
    std::cout << "Canvas redraw state = true!\n";
}


std::function<void()> reactorOnUpdated = RedrawCanvas;
void ReactorUpdate() {
    std::cout << "Isolate Reactor has updated!\n";
    reactorOnUpdated();
}

void ReactorAddMolecule() {
    reactorOnUpdated();
}

void ButtonFunction() {
    std::cout << "I am pressed!\n"; 
}


void AddMoleculeHandler() {
    std::cout << "ReactorAddMolecule\n";
    ReactorAddMolecule();
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
    MGWindow *window1 = mainWindow->addWindow(0, 0, 255, 255);
    MGWindow *window2 = mainWindow->addWindow(300, 300, 400, 400);
    MGCanvas *canvas = window1->addCanvas(30, 30, 1000, 1000);
    window2->addCanvas(60, 60, 300, 300);
    MGButton *button = window2->addButton(30, 30, 100, 100, PRESSED_BUTTON_IMAGE_PATH, UNPRESSED_BUTTON_IMAGE_PATH, AddMoleculeHandler);

    // button->connectSignal("AddMolecule", AddMoleculeHandler);
    application.addEventToMainLoop(ReactorUpdate);

    // signalManager->connect("reactor_update", ReactorUpdate);
    // signalManager->connect("reactor_update", RedrawCanvas);

    // application.addEventToMainLoop(ReactorUpdate);


    application.run();


    return 0;
}