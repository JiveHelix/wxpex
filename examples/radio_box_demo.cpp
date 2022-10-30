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
#include <pex/select.h>

#include "wxpex/view.h"
#include "wxpex/radio_box.h"
#include "wxpex/view.h"

#include "units.h"


using Select = pex::model::Select<UnitSystem>;
using SelectControl = pex::control::Select<void, Select>;


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        select_{
            UnitSystem::MKS,
            {
                UnitSystem::MKS,
                UnitSystem::CGS,
                UnitSystem::FPS,
                UnitSystem::FFF}}
    {

    }

    bool OnInit() override;

private:
    Select select_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(SelectControl select);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(SelectControl(this->select_));

    exampleFrame->Show();
    return true;
}


ExampleFrame::ExampleFrame(SelectControl select)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::RadioBox Demo")
{
    auto radioBox =
        new wxpex::RadioBox<SelectControl, ShortConverter>(
            this,
            select,
            "Choose Units");

    auto view =
        new wxpex::View<SelectControl::Value, LongConverter>(
            this,
            select.value);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(radioBox, 0, wxALL, 10);
    topSizer->Add(view, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);

    this->SetSizerAndFit(topSizer.release());
}
