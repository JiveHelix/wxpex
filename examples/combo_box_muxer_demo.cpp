/**
  * @file combo_box_demo.cpp
  *
  * @brief A demonstration of wxpex::ComboBox, backed by
  * a pex ChoiceMuxer.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 07 Oct 2025
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <string>
#include <vector>
#include <memory>

#include <pex/choice_muxer.h>

#include "wxpex/combo_box.h"
#include "wxpex/check_box.h"
#include "wxpex/view.h"

#include "units.h"


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        unitsMuxer_{}
    {
        PEX_NAME("ExampleApp");
        PEX_MEMBER(unitsMuxer_);
    }

    bool OnInit() override;

private:
    UnitsMuxerModel unitsMuxer_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(UnitsMuxerControl unitsMuxer);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(UnitsMuxerControl(this->unitsMuxer_));

    exampleFrame->Show();
    return true;
}


ExampleFrame::ExampleFrame(UnitsMuxerControl unitsMuxer)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::ComboBox Demo")
{
    auto firkinsCheckbox =
        new wxpex::CheckBox(
            this,
            "Show FFF",
            unitsMuxer.key);

    auto shortComboBox =
        wxpex::MakeComboBox<ShortConverter>(this, unitsMuxer.select);

    auto longComboBox =
        wxpex::MakeComboBox<LongConverter>(this, unitsMuxer.select);

    using OptionalSelect = decltype(UnitsMuxerControl::select);

    auto shortView =
        new wxpex::View<OptionalSelect::Value, ShortConverter>(
            this,
            unitsMuxer.select.value);

    auto longView =
        new wxpex::View<OptionalSelect::Value, LongConverter>(
            this,
            unitsMuxer.select.value);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(firkinsCheckbox, 0, wxALL, 10);
    topSizer->Add(shortComboBox, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(longComboBox, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(shortView, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(longView, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);

    this->SetSizerAndFit(topSizer.release());
}
