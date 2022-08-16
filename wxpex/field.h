/**
  * @file field.h
  *
  * @brief A field for textual or numeric entry, connected to
  * a pex::control::Value.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 08 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <pex/value.h>
#include <pex/converter.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template
<
    typename Control,
    typename ConverterTraits = pex::DefaultConverterTraits
>
class Field: public wxControl
{
public:
    using Base = wxControl;
    using Type = typename Control::Type;
    using Convert = pex::Converter<Type, ConverterTraits>;

    Field(
        wxWindow *parent,
        Control value,
        long style = 0)
        :
        Base(parent, wxID_ANY),
        value_{this, value},
        displayedString_{Convert::ToString(this->value_.Get())},
        textControl_(
            new wxTextCtrl(
                this,
                wxID_ANY,
                this->displayedString_,
                wxDefaultPosition,
                wxDefaultSize,
                style | wxTE_PROCESS_ENTER,
                wxDefaultValidator))
    {
        this->textControl_->Bind(wxEVT_TEXT_ENTER, &Field::OnEnter_, this);
        this->textControl_->Bind(wxEVT_KILL_FOCUS, &Field::OnKillFocus_, this);

        PEX_LOG("Connect");
        this->value_.Connect(&Field::OnValueChanged_);
    }

    // NOTE: As of wx 3.1.3, changing the font size does not affect
    // wxTextCtrl's GetBestSize, defeating the window layout mechanism.
    bool SetFont(const wxFont &font) override
    {
        return this->textControl_->SetFont(font);
    }

private:
    void OnEnter_(wxCommandEvent &)
    {
        this->ProcessUserInput_();
    }

    void OnKillFocus_(wxEvent &event)
    {
        this->ProcessUserInput_();
        event.Skip();
    }

    void ProcessUserInput_()
    {
        auto userInput = this->textControl_->GetValue().ToStdString();

        if (userInput == this->displayedString_)
        {
            // value has not changed.
            // Ignore this input.
            return;
        }

        try
        {
            this->value_.Set(Convert::ToValue(userInput));
        }
        catch (std::out_of_range &)
        {
            this->textControl_->ChangeValue(this->displayedString_);
        }
        catch (std::invalid_argument &)
        {
            this->textControl_->ChangeValue(this->displayedString_);
        }
    }

    void OnValueChanged_(pex::Argument<Type> value)
    {
        this->displayedString_ = Convert::ToString(value);
        this->textControl_->ChangeValue(this->displayedString_);
    }

    using Value = pex::Terminus<Field, Control>;

    Value value_;
    std::string displayedString_;
    wxTextCtrl *textControl_;
};


} // namespace wxpex
