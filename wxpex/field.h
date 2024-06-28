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


struct FixedFieldTag {};


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

private:
    // long and bool are ambiguous types.
    // Use tag dispatch to create a fixed-width Field.
    // Existing uses of this class can continue to use the orginal constructor
    // without modification. The default behavior is to set the minimum width
    // to match the field text.
    Field(
        wxWindow *parent,
        Control value,
        bool fixedWidth,
        long style)
        :
        Base(parent, wxID_ANY),
        fixedWidth_(fixedWidth),
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
                wxDefaultValidator))
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

        this->SetMinClientSize(ToWxSize(this->GetFittingSize()));
    }

public:
    Field(
        wxWindow *parent,
        Control value,
        const FixedFieldTag &,
        long style = 0)
        :
        Field(parent, value, true, style)
    {

    }

    Field(
        wxWindow *parent,
        Control value,
        long style = 0)
        :
        Field(parent, value, false, style)
    {

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

        if (this->fixedWidth_)
        {
            fittingSize.width = this->minimumWidth_;
        }
        else
        {
            fittingSize.width =
                std::max(this->minimumWidth_, fittingSize.width);
        }

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
        return ToWxSize(this->GetFittingSize());
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

#ifdef __WXGTK__
        // In GTK, ChangeValue does not reliably draw the new text in the
        // control. A call to Update forces it.
        this->textControl_->Update();
#endif

        if (!this->fixedWidth_)
        {
            this->UpdateMinimumSize_();
        }
    }

    void UpdateMinimumSize_()
    {
        // Text entry field should resize to fit whatever text is displayed.
        // TODO: Create a version that allows fixed size text entry fields.

        auto fitting = this->GetFittingSize();
        this->SetMinClientSize(ToWxSize(fitting));
        this->InvalidateBestSize();
    }

    using Value = pex::Terminus<Field, Control>;

    bool fixedWidth_;
    int minimumWidth_;
    Value value_;
    std::string displayedString_;
    wxTextCtrl *textControl_;
};


template<typename Control, typename Traits = pex::DefaultConverterTraits>
wxWindow * MakeOptionalField(wxWindow *parent, Control control)
{
    using OptionalType = typename Control::Type;
    using ValueType = typename OptionalType::value_type;

    using OptionalField =
        Field<Control, pex::OptionalConverter<ValueType, Traits>>;

    return new OptionalField(parent, control);
}


} // namespace wxpex
