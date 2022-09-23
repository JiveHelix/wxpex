/**
  * @file radio_box_demo.cpp
  *
  * @brief Demonstrates the usage of wxpex::RadioBox.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 07 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <memory>
#include <pex/chooser.h>

#include "wxpex/view.h"
#include "wxpex/radio_box.h"
#include "wxpex/view.h"

#include "units.h"


using Chooser = pex::model::Chooser<UnitsModel, pex::GetTag>;
using ChooserControl = pex::control::Chooser<void, Chooser>;


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        units_{UnitSystem::MKS},
        chooser_{
            this->units_,
            {
                UnitSystem::MKS,
                UnitSystem::CGS,
                UnitSystem::FPS,
                UnitSystem::FFF}}
    {

    }

    bool OnInit() override;

private:
    UnitsModel units_;
    Chooser chooser_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(ChooserControl chooser);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(ChooserControl(this->chooser_));

    exampleFrame->Show();
    return true;
}


ExampleFrame::ExampleFrame(ChooserControl chooserControl)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::RadioBox Demo")
{
    auto radioBox =
        new wxpex::RadioBox<ChooserControl, ShortConverter>(
            this,
            chooserControl,
            "Choose Units");

    auto view =
        new wxpex::View<ChooserControl::Value, LongConverter>(
            this,
            chooserControl.value);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(radioBox, 0, wxALL, 10);
    topSizer->Add(view, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);

    this->SetSizerAndFit(topSizer.release());
}
