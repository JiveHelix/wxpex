/**
  * @file combo_box_demo.cpp
  *
  * @brief A demonstration of wxpex::ComboBox, backed by
  * a pex::control::Select.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 14 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <string>
#include <vector>
#include <memory>

#include <pex/select.h>

#include "wxpex/combo_box.h"
#include "wxpex/check_box.h"
#include "wxpex/view.h"

#include "units.h"


static inline const std::vector<UnitSystem> withoutFirkins
{
    UnitSystem::MKS,
    UnitSystem::CGS,
    UnitSystem::FPS
};


static inline const std::vector<UnitSystem> withFirkins
{
    UnitSystem::MKS,
    UnitSystem::CGS,
    UnitSystem::FPS,
    UnitSystem::FFF
};


std::string fffUnits = "furlong-firkin-fortnight";

using Select = pex::model::Select<UnitSystem>;
using SelectControl = pex::control::Select<Select>;

using EnableFirkins = pex::model::Value<bool>;
using EnableFirkinsControl = pex::control::Value<EnableFirkins>;


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        select_{
            UnitSystem::MKS,
            withoutFirkins},
        enableFirkins_(false)
    {
        this->enableFirkins_.Connect(this, &ExampleApp::OnFirkins_);
    }

    ~ExampleApp()
    {
        this->enableFirkins_.Disconnect(this);
    }

    bool OnInit() override;

private:
    static void OnFirkins_(void * context, bool firkins)
    {
        auto self = static_cast<ExampleApp *>(context);

        if (firkins)
        {
            self->select_.SetChoices(withFirkins);
        }
        else
        {
            self->select_.SetChoices(withoutFirkins);
        }
    }

    Select select_;
    EnableFirkins enableFirkins_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(
        SelectControl select,
        EnableFirkinsControl enableFirkinsControl);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(
            SelectControl(this->select_),
            EnableFirkinsControl(this->enableFirkins_));

    exampleFrame->Show();
    return true;
}


ExampleFrame::ExampleFrame(
    SelectControl select,
    EnableFirkinsControl enableFirkinsControl)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::ComboBox Demo")
{
    auto firkinsCheckbox =
        new wxpex::CheckBox(
            this,
            "Show FFF",
            enableFirkinsControl);

    auto shortComboBox =
        new wxpex::ComboBox<SelectControl, ShortConverter>(
            this,
            select);

    auto longComboBox =
        new wxpex::ComboBox<SelectControl, LongConverter>(
            this,
            select);

    auto shortView =
        new wxpex::View<SelectControl::Value, ShortConverter>(
            this,
            select.value);

    auto longView =
        new wxpex::View<SelectControl::Value, LongConverter>(
            this,
            select.value);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(firkinsCheckbox, 0, wxALL, 10);
    topSizer->Add(shortComboBox, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(longComboBox, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(shortView, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    topSizer->Add(longView, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);

    this->SetSizerAndFit(topSizer.release());
}
