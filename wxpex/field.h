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
#include "wxpex/size.h"


namespace wxpex
{


template
<
    typename Control,
    typename Convert = pex::Converter<typename Control::Type>
>
class Field: public wxControl
{
public:
    static constexpr auto observerName = "wxpex::Field";

    using Base = wxControl;
    using Type = typename Control::Type;

    Field(
        wxWindow *parent,
        Control value,
        long style = 0)
        :
        Base(parent, wxID_ANY),
        minimumWidth_{},
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
                wxDefaultValidator)),
        fittingSize_{this->GetFittingSize()}
    {
        this->textControl_->Bind(wxEVT_TEXT_ENTER, &Field::OnEnter_, this);
        this->textControl_->Bind(wxEVT_KILL_FOCUS, &Field::OnKillFocus_, this);

        PEX_LOG("Connect");
        this->value_.Connect(&Field::OnValueChanged_);

        // A sizer is required to allow the text control to be managed by
        // a hierarchy of sizers.
        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(this->textControl_, 1, wxEXPAND);
        this->SetSizerAndFit(sizer.release());

        this->SetMinClientSize(ToWxSize(this->fittingSize_));
    }

    // NOTE: As of wx 3.1.3, changing the font size does not affect
    // wxTextCtrl's GetBestSize, defeating the window layout mechanism.
    bool SetFont(const wxFont &font) override
    {
        bool result = this->textControl_->SetFont(font);
        this->UpdateMinimumSize_();

        return result;
    }

    tau::Size<int> GetFittingSize() const
    {
        auto fittingSize = ToSize<int>(
            this->textControl_->GetSizeFromTextSize(
                this->textControl_->GetTextExtent(
                    Convert::ToString(this->value_.Get()))));

        fittingSize.width = std::max(this->minimumWidth_, fittingSize.width);

        return fittingSize;
    }

    void SetMinimumWidth(int width)
    {
        this->minimumWidth_ = width;
        this->UpdateMinimumSize_();
    }

protected:
    wxSize DoGetBestClientSize() const override
    {
        this->fittingSize_ = this->GetFittingSize();
        return ToWxSize(this->fittingSize_);
    }

private:
    void OnEnter_(wxCommandEvent &event)
    {
        this->ProcessUserInput_();
        event.Skip();
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

        this->GetParent()->Layout();
    }

    void OnValueChanged_(pex::Argument<Type> value)
    {
        this->displayedString_ = Convert::ToString(value);
        this->textControl_->ChangeValue(this->displayedString_);
        this->UpdateMinimumSize_();

#ifdef __WXGTK__
        // In GTK, ChangeValue does not reliably draw the new text in the
        // control. A call to Update forces it.
        this->textControl_->Update();
#endif
    }

    void UpdateMinimumSize_()
    {
        // Text entry field should resize to fit whatever text is displayed.
        // TODO: Create a version that allows fixed size text entry fields.

        auto fitting = this->GetFittingSize();
        this->fittingSize_ = fitting;
        this->SetMinClientSize(ToWxSize(fitting));
        this->InvalidateBestSize();
    }

    using Value = pex::Terminus<Field, Control>;

    int minimumWidth_;
    Value value_;
    std::string displayedString_;
    wxTextCtrl *textControl_;
    mutable tau::Size<int> fittingSize_;
};


} // namespace wxpex
