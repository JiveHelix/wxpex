/**
  * @file check_box.h
  *
  * @brief A CheckBox backed by a pex::control::Value.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 07 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include "wxpex/wxshim.h"
#include <pex/value.h>


namespace wxpex
{


template<typename Value>
class CheckBox: public wxCheckBox
{
public:
    using Base = wxCheckBox;
    using Type = typename Value::Type;

    CheckBox(
        wxWindow *parent,
        const std::string &label,
        Value value,
        long style = 0)
        :
        Base(
            parent,
            wxID_ANY,
            label,
            wxDefaultPosition,
            wxDefaultSize,
            style),
        value_(this, value)
    {
        this->SetValue(this->value_.Get());

        PEX_LOG("Connect");
        this->value_.Connect(&CheckBox::OnValueChanged_);

        this->Bind(wxEVT_CHECKBOX, &CheckBox::OnCheckBox_, this);
    }

    void OnValueChanged_(Type value)
    {
        this->SetValue(value);
    }

    void OnCheckBox_(wxCommandEvent &event)
    {
        this->value_.Set(event.IsChecked());
    }

private:
    pex::Terminus<CheckBox, Value> value_;
    size_t id_;
};


} // namespace wxpex
