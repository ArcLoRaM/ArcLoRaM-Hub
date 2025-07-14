#include "UIFactory.hpp"
void UIFactory::setTheme(tgui::Theme* theme) {
    s_theme = theme;
}

tgui::Button::Ptr UIFactory::createButton(const std::string &text, const std::function<void()> &callback)
{
    auto button = tgui::Button::create(text);
    if (s_theme) button->setRenderer(s_theme->getRenderer("Button"));
    if (callback) button->onPress(callback);
    return button;
}
tgui::ToggleButton::Ptr UIFactory::createToggleButton(const std::string& text) {
    auto button = tgui::ToggleButton::create(text);
    if (s_theme)
        button->setRenderer(s_theme->getRenderer("ToggleButton"));
    return button;
}
    
tgui::EditBox::Ptr UIFactory::createTypeableInput(const std::string& defaultText, const tgui::Layout2d& size) {
    auto editBox = tgui::EditBox::create();
    editBox->setSize(size);
    editBox->setDefaultText(defaultText);
    if (s_theme) editBox->setRenderer(s_theme->getRenderer("EditBox"));
    return editBox;
}

tgui::RadioButtonGroup::Ptr UIFactory::createRadioGroup() {
    return tgui::RadioButtonGroup::create();
}

tgui::RadioButton::Ptr UIFactory::createRadioButton(const std::string& text, tgui::RadioButtonGroup::Ptr group) {

    auto radio = tgui::RadioButton::create();
    radio->setText(text);
    if (s_theme) radio->setRenderer(s_theme->getRenderer("RadioButton"));
    if (group) group->add(radio);

    return radio;
}

tgui::ComboBox::Ptr UIFactory::createEnumComboBox(const tgui::Layout2d& size) {
    auto comboBox = tgui::ComboBox::create();
    comboBox->setSize(size);
    if (s_theme) comboBox->setRenderer(s_theme->getRenderer("ComboBox"));
    return comboBox;
}

void UIFactory::applyRenderer(tgui::Widget::Ptr widget, const std::string& section) {
    if (s_theme && s_theme->getRenderer(section))
        widget->setRenderer(s_theme->getRenderer(section));
}


tgui::Label::Ptr UIFactory::createLabel(const std::string& text) {
    auto label = tgui::Label::create(text);
    if (s_theme) label->setRenderer(s_theme->getRenderer("Label"));
    return label;
}