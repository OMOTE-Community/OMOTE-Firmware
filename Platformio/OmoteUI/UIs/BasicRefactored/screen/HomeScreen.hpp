#pragma once
#include "HardwareAbstract.hpp"
#include "PageBase.hpp"
#include "ScreenBase.hpp"
#include "TabView.hpp"
#include <string>
namespace UI::Screen {

class HomeScreen : public Base {
public:
  HomeScreen();

  void SetBgColor(lv_color_t value,
                  lv_style_selector_t selector = LV_PART_MAIN) override;

  void AddPage(Page::Base::Ptr aPage);

  bool GoToPage(ID anId) { return mTabView->GoToTab(anId); }

protected:
  bool OnKeyEvent(KeyPressAbstract::KeyEvent aKeyEvent) override;

private:
  Page::TabView *mTabView;
};

} // namespace UI::Screen