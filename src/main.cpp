#include "core/Application.h"

int main(int argc, char* args[]) {
    Application app;

    if (app.init()) {
        app.run();
    }

    return 0;
}