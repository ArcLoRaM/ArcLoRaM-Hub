#pragma once

#include <TGUI/TGUI.hpp>
#include <string>
#include <functional>

class UIFactory {
public:
    static void setTheme(tgui::Theme* theme);

    // Common widgets
   static tgui::Button::Ptr createButton(const std::string& text, const std::function<void()>& callback = {});
   static tgui::EditBox::Ptr createTypeableInput(const std::string& defaultText = "", const tgui::Layout2d& size = {"40%", "6%"});
    static tgui::RadioButton::Ptr createRadioButton(const std::string& text, tgui::RadioButtonGroup::Ptr group = nullptr);
    static tgui::RadioButtonGroup::Ptr createRadioGroup();
    static tgui::ComboBox::Ptr createEnumComboBox(const tgui::Layout2d& size = {"30%", "5%"});
    static tgui::Label::Ptr createLabel(const std::string& text);
    static tgui::ToggleButton::Ptr createToggleButton(const std::string& text);
    static tgui::MessageBox::Ptr createMessageBox(const std::string& title, const std::string& message, const std::vector<std::string>& buttons = {"OK"});
    // tgui::Label::Ptr createLabel(const std::string& text);
    // tgui::EditBox::Ptr createEditBox(const std::string& defaultText = "");
    // tgui::CheckBox::Ptr createCheckBox(const std::string& label, bool defaultState = false);
    // tgui::Slider::Ptr createSlider(float min, float max, float value);
    // tgui::Panel::Ptr createPanel(const tgui::Layout2d& size);

    static void applyRenderer(tgui::Widget::Ptr widget, const std::string& section);

    static tgui::ChildWindow::Ptr createEditWithLabelPopup(
        const std::string& title,
        const std::string& labelText,
        const std::string& defaultText ,
        const std::string&buttonText,
        const std::function<void(const std::string&)>& callback);
private:
    static inline tgui::Theme* s_theme = nullptr; // pointer to shared theme
};
