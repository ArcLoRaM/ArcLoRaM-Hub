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

tgui::MessageBox::Ptr UIFactory::createMessageBox(const std::string &title, const std::string &message, const std::vector<std::string> &buttons)
{
    auto messageBox = tgui::MessageBox::create(title, message);
    for( const auto &button : buttons) {
        messageBox->addButton(button);
    }
    if(s_theme) {
        messageBox->setRenderer(s_theme->getRenderer("MessageBox"));
    }
    messageBox->setButtonAlignment(tgui::HorizontalAlignment::Right);
    messageBox->setSize({"60%", "40%"});
    messageBox->setPosition({"20%", "20%"});
    return messageBox;
}

tgui::TabContainer::Ptr UIFactory::createTabContainer(const tgui::Layout2d &size)
{
    auto tabContainer = tgui::TabContainer::create();
    tabContainer->setSize(size);
    // if (s_theme) tabContainer->setRenderer(s_theme->getRenderer("TabContainer"));
    tabContainer->setTabAlignment(tgui::TabContainer::TabAlign::Top); // Tabs above panels
    return tabContainer;
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

tgui::ChildWindow::Ptr UIFactory::createEditWithLabelPopup(const std::string &title, const std::string &labelText, const std::string &defaultText,const std::string&buttonText,const std::function<void(const std::string&)>& callback)
{
   auto popup = tgui::ChildWindow::create(title);
   popup->setTitleButtons(tgui::ChildWindow::TitleButton::Close);

    popup->setSize({"35%", "18%"});
    popup->setPosition({"32.5%", "41%"});
    popup->setTitleAlignment(tgui::ChildWindow::TitleAlignment::Center);
    popup->setResizable(true);

    auto editBox = createTypeableInput(defaultText,{"60%", "25%"});
    editBox->setDefaultText("Enter text...");
    editBox->setText(defaultText);
    editBox->setPosition({"5%", "25%"});
    popup->add(editBox);

    auto label = createLabel(labelText);
    label->setSize({"30%", "25%"});
    label->setPosition({"70%", "25%"});
    label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
    label->setText(".simcfg");
    popup->add(label);


    // Create Button
    auto button = createButton(buttonText, [editBox, callback]() {
        callback(editBox->getText().toStdString());
    });
    button->setSize({"30%", "25%"});
    button->setPosition({"35%", "65%"});
    popup->add(button);
    
    return popup;
}

tgui::Label::Ptr UIFactory::createLabel(const std::string& text) {
    auto label = tgui::Label::create(text);
    if (s_theme) label->setRenderer(s_theme->getRenderer("Label"));
    return label;
}