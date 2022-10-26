/**
  * @file spin_control_double_demo.cpp
  *
  * @brief A demonstration of wxpex::SpinControlDouble.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 17 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <pex/range.h>
#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/spin_control.h"


using pex::Limit;


using Wibble = pex::model::Range<double, Limit<0>, Limit<20>>;
using WibbleControl = pex::control::Range<void, Wibble>;
using WibbleValue = typename WibbleControl::Value;

inline constexpr double defaultWibble = 10;
inline constexpr double wibbleIncrement = 1;


using Wobble = pex::model::Range<float, Limit<-100>, Limit<100>>;
using WobbleControl = pex::control::Range<void, Wobble>;
using WobbleValue = typename WobbleControl::Value;

inline constexpr float defaultWobble = 0;
inline constexpr float wobbleIncrement = 2.5;


class ExampleApp : public wxApp
{
public:
    ExampleApp()
        :
        wibble_(defaultWibble),
        wobble_(defaultWobble)
    {

    }

    bool OnInit() override;

private:
    Wibble wibble_;
    Wobble wobble_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(
        WibbleControl wibbleControl,
        WobbleControl wobbleControl);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(
            WibbleControl(this->wibble_),
            WobbleControl(this->wobble_));

    exampleFrame->Show();

    return true;
}


using WibbleSpinControl = wxpex::SpinControl<WibbleControl>;
using WobbleSpinControl = wxpex::SpinControl<WobbleControl>;


ExampleFrame::ExampleFrame(WibbleControl wibble, WobbleControl wobble)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::SpinControl Demo")
{
    auto wibbleView =
        new wxpex::View(this, wibble.value);

    auto wibbleSpinControl =
        new WibbleSpinControl(this, wibble, wibbleIncrement, 0);

    auto wobbleView =
        new wxpex::View(this, wobble.value);

    auto wobbleSpinControl =
        new WobbleSpinControl(this, wobble, wobbleIncrement);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(wibbleSpinControl, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(wibbleView, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(wobbleSpinControl, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(wobbleView, 0, wxALL | wxEXPAND, 10);
    this->SetSizerAndFit(topSizer.release());
}
