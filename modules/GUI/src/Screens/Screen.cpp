#include "Screen.hpp"

void Screen::onResize() {
    updateTextSize();  // default behavior
}

void Screen::updateTextSize(float percentage) {
    float windowHeight = gui.getView().getRect().height;
    gui.setTextSize(static_cast<unsigned int>(percentage* windowHeight));
}