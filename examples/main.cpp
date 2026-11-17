#include <MadDisplay/buffer.hpp>
#include <MadDisplay/components.hpp>
#include <MadDisplayWebDebugger/MadDisplayWebDebugger.hpp>

#include <cstdio>
#include <cstring>
#include <iostream>

namespace {

struct PageLink { mad::Compositor* compositor; mad::Compositor::PageId page; };

class HardwareMetricLabel final : public mad::Component {
 public:
  enum class Metric { FrameTime, Fps };

  HardwareMetricLabel(mad::Compositor& compositor, Metric metric)
      : compositor_(compositor), metric_(metric), text_{} {}

  mad::Size preferredSize() const override {
    updateText();
    return {static_cast<int16_t>(std::strlen(text_) * mad::kTextAdvance), mad::kTextHeight};
  }

  void draw(mad::Canvas& canvas) const override {
    updateText();
    canvas.text(bounds_.x, bounds_.y, text_, 1, mad::Color::On);
  }

 private:
  void updateText() const {
    if (compositor_.actualFrameTimeMs() == 0) {
      std::snprintf(text_, sizeof(text_), metric_ == Metric::FrameTime ? "FRAME TIME: -- ms" : "FPS: --");
    } else if (metric_ == Metric::FrameTime) {
      std::snprintf(text_, sizeof(text_), "FRAME TIME: %u ms", compositor_.actualFrameTimeMs());
    } else {
      std::snprintf(text_, sizeof(text_), "FPS: %u", compositor_.actualFps());
    }
  }

  mad::Compositor& compositor_;
  Metric metric_;
  mutable char text_[32];
};

class ExampleUi {
 public:
  ExampleUi() : page2Link_{&compositor_, mad::Compositor::kInvalidPage} {
    compositor_.setFps(60);
    topLayout_.setWidthMode(mad::SizeMode::Fit);
    secondLayout_.setWidthMode(mad::SizeMode::Fit);
    topLayout_.add(button1_);
    topLayout_.add(button2_);
    secondLayout_.add(label_);
    secondLayout_.add(openPage2Button_);
    openPage2Button_.onUse(openPage2, &page2Link_);
    page1Root_.add(topLayout_);
    page1Root_.add(secondLayout_);
    page2Panel_.add(page2Label_);
    page2Panel_.add(page2FrameDelayLabel_);
    page2Panel_.add(page2FpsLabel_);
    page2Panel_.add(toggleLabel_);
    toggleLayout_.add(demoSwitch_);
    toggleLayout_.add(demoCheckbox_);
    demoRadioA_.join(radioGroup_);
    demoRadioB_.join(radioGroup_);
    toggleLayout_.add(demoRadioA_);
    toggleLayout_.add(demoRadioB_);
    page2Panel_.add(toggleLayout_);
    page2Root_.add(page2Panel_);
    page1Id_ = compositor_.registerPage(page1_);
    page2Id_ = compositor_.registerPage(page2_);
    page2Link_.page = page2Id_;
    compositor_.openPage(page1Id_);
  }

  mad::Compositor& compositor() { return compositor_; }

 private:
  static void openPage2(void* context) {
    PageLink* link = static_cast<PageLink*>(context);
    link->compositor->openPage(link->page);
  }

  mad::Compositor compositor_;
  mad::VerticalLayout page1Root_;
  mad::VerticalLayout page2Root_;
  mad::Page page1_{page1Root_};
  mad::Page page2_{page2Root_};
  mad::HorizontalLayout topLayout_;
  mad::Button button1_{"BUTTON 1"};
  mad::Button button2_{"BUTTON 2"};
  mad::HorizontalLayout secondLayout_;
  mad::Label label_{"LABEL"};
  mad::Button openPage2Button_{"PAGE 2"};
  mad::Panel page2Panel_;
  mad::Label page2Label_{"Page2"};
  HardwareMetricLabel page2FrameDelayLabel_{compositor_, HardwareMetricLabel::Metric::FrameTime};
  HardwareMetricLabel page2FpsLabel_{compositor_, HardwareMetricLabel::Metric::Fps};
  mad::Label toggleLabel_{"TOGGLES"};
  mad::HorizontalLayout toggleLayout_;
  mad::Switch demoSwitch_{false};
  mad::Checkbox demoCheckbox_{false};
  mad::RadioGroup radioGroup_;
  mad::RadioButton demoRadioA_{true};
  mad::RadioButton demoRadioB_{false};
  mad::Compositor::PageId page1Id_;
  mad::Compositor::PageId page2Id_;
  PageLink page2Link_;
};

}  // namespace

int main() {
  mad::BareBuffer<> buffer;
  ExampleUi ui;
  madweb::MadDisplayWebDebugger debugger(buffer, ui.compositor());
  if (!debugger.start()) return 1;
  std::cout << "MadDisplay Web Debugger: http://127.0.0.1:4174/\n" << std::flush;
  debugger.run();
  return 0;
}
