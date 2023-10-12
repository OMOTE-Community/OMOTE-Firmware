#pragma once
#include "WidgetBase.hpp"

namespace UI::Widget{

class ListItem : public UIElement{
public:
    ListItem(lv_obj_t* aListItem, std::function<void()> onItemSelected);
protected:
    void OnLvglEvent(lv_event_t* anEvent) override;
    bool OnKeyEvent(KeyPressAbstract::KeyEvent anEvent)override{return false;};
    void OnShow()override{};
    void OnHide()override{};
private:
    std::function<void()> mSelectedHandler;
};

class List : public Base{
public:
    List();
    void AddItem(std::string aTitle, const char* aSymbol, std::function<void()> onItemSelected);

protected:
    void OnLvglEvent(lv_event_t* anEvent)override;

private:
    std::vector<UIElement::Ptr> mListItems;
};

}