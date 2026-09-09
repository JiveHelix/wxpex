/**
  * @file combo_box.h
  *
  * @brief A read-only wxComboBox backed by pex::Select.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 12 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <pex/select.h>
#include <pex/endpoint.h>

#include "wxpex/wxshim.h"
#include "wxpex/wx_select.h"


namespace wxpex
{


template
<
    typename Control,
    typename Convert = pex::Converter<typename Control::Type>
>
class ComboBox : public wxComboBox
{
public:
    static constexpr auto observerName = "wxpex::ComboBox";

    using Base = wxComboBox;
    using This = ComboBox<Control, Convert>;

    using Selection = pex::Endpoint<ComboBox, typename Control::Selection>;
    using Choices = pex::Endpoint<ComboBox, typename Control::Choices>;
    using ChoicesVector = typename Choices::Type;
    using ValueType = typename Control::Type;
    using WxAdapter = WxSelect<ValueType, Convert>;

    ComboBox(
        wxWindow *parent,
        const Control &control,
        long style = 0)
        :
        Base(
            parent,
            wxID_ANY,
            WxAdapter::GetSelectionAsString(
                control.selection.Get(),
                control.choices.Get()),
            wxDefaultPosition,
            wxDefaultSize,
            WxAdapter::GetChoicesAsStrings(control.choices.Get()),
            style | wxCB_READONLY),

        selection_(
            PEX_THIS("wxpex::ComboBox"),
            control.selection,
            &ComboBox::OnSelectionChanged_),

        choices_(
            this,
            control.choices,
            &ComboBox::OnChoicesChanged_)
    {
        this->Bind(wxEVT_COMBOBOX, &ComboBox::OnComboBox_, this);
    }

private:
    void OnSelectionChanged_(size_t index)
    {
        this->SetValue(
            WxAdapter::GetSelectionAsString(index, this->choices_.Get()));
    }

    void OnChoicesChanged_(const ChoicesVector &choices)
    {
        this->Set(WxAdapter::GetChoicesAsStrings(choices));
        this->OnSelectionChanged_(this->selection_.Get());
    }

    void OnComboBox_(wxCommandEvent &event)
    {
        auto index = event.GetSelection();

        if (index < 0)
        {
            return;
        }

        this->selection_.Set(static_cast<size_t>(index));
    }

    Selection selection_;
    Choices choices_;
};


template
<
    typename Control,
    typename Convert = pex::Converter<typename Control::Type>
>
class ReadOnlySelect : public wxStaticText
{
public:
    static constexpr auto observerName = "wxpex::ReadOnlySelect";

    using Base = wxStaticText;
    using This = ReadOnlySelect<Control, Convert>;

    using Selection =
        pex::Endpoint<ReadOnlySelect, typename Control::Selection>;

    using Choices = pex::Endpoint<ReadOnlySelect, typename Control::Choices>;
    using ChoicesVector = typename Choices::Type;
    using ValueType = typename Control::Type;
    using WxAdapter = WxSelect<ValueType, Convert>;

    ReadOnlySelect(
        wxWindow *parent,
        const Control &control,
        long style = 0)
        :
        Base(
            parent,
            wxID_ANY,
            WxAdapter::GetSelectionAsString(
                control.selection.Get(),
                control.choices.Get()),
            wxDefaultPosition,
            wxDefaultSize),

        selection_(
            PEX_THIS("wxpex::ReadOnlySelect"),
            control.selection,
            &ReadOnlySelect::OnSelectionChanged_),

        choices_(
            this,
            control.choices,
            &ReadOnlySelect::OnChoicesChanged_)
    {

    }

private:
    void OnSelectionChanged_(size_t index)
    {
        this->SetLabel(
            WxAdapter::GetSelectionAsString(index, this->choices_.Get()));

        this->UpdateMinimumSize_();
        this->GetParent()->Layout();
    }

    void UpdateMinimumSize_()
    {
        // Text field should resize to fit whatever text is displayed.
        auto fittingSize =
            this->GetSizeFromTextSize(
                this->GetTextExtent(
                    WxAdapter::GetSelectionAsString(
                        this->selection_.Get(),
                        this->choices_.Get())));

        this->SetMinClientSize(fittingSize);
        this->InvalidateBestSize();
    }

    void OnChoicesChanged_(const ChoicesVector &choices)
    {
        this->OnSelectionChanged_(this->selection_.Get());
    }


    Selection selection_;
    Choices choices_;
};


template
<
    typename Control,
    typename Convert = pex::Converter<typename Control::Type>
>
class OptionalComboBox : public wxComboBox
{
public:
    static_assert(
        jive::IsOptional<typename Control::Selection::Type>,
        "Expects the Selection type to be std::optional");

    static constexpr auto observerName = "wxpex::OptionalComboBox";

    using Base = wxComboBox;
    using This = OptionalComboBox<Control, Convert>;

    using Selection = pex::Endpoint<OptionalComboBox, typename Control::Selection>;
    using Choices = pex::Endpoint<OptionalComboBox, typename Control::Choices>;
    using ChoicesVector = typename Choices::Type;
    using ValueType = typename Control::Type;
    using WxAdapter = WxSelect<ValueType, Convert>;

    OptionalComboBox(
        wxWindow *parent,
        Control control,
        std::string_view nullValue = "",
        long style = 0)
        :
        Base(
            parent,
            wxID_ANY,
            WxAdapter::GetSelectionAsString(
                control.selection.Get(),
                control.choices.Get(),
                nullValue),
            wxDefaultPosition,
            wxDefaultSize,
            WxAdapter::GetChoicesAsStrings(control.choices.Get(), nullValue),
            style | wxCB_READONLY),

        nullValue_(nullValue),

        selection_(
            PEX_THIS("wxpex::OptionalComboBox"),
            control.selection,
            &OptionalComboBox::OnSelectionChanged_),

        choices_(
            this,
            control.choices,
            &OptionalComboBox::OnChoicesChanged_)
    {
        this->Bind(wxEVT_COMBOBOX, &OptionalComboBox::OnComboBox_, this);
    }

private:
    void OnSelectionChanged_(const std::optional<size_t> &index)
    {
        this->SetValue(
            WxAdapter::GetSelectionAsString(
                index,
                this->choices_.Get(),
                this->nullValue_));
    }

    void OnChoicesChanged_(const ChoicesVector &choices)
    {
        this->Set(WxAdapter::GetChoicesAsStrings(choices, this->nullValue_));
        this->OnSelectionChanged_(this->selection_.Get());
    }

    void OnComboBox_(wxCommandEvent &event)
    {
        auto index = event.GetSelection();

        if (index < 0)
        {
            return;
        }

        // TODO: choices_ should support size() directly.
        if (static_cast<size_t>(index) == this->choices_.Get().size())
        {
            // The user has selected the null value.
            this->selection_.Set(std::nullopt);
        }
        else
        {
            this->selection_.Set(static_cast<size_t>(index));
        }
    }

    std::string_view nullValue_;
    Selection selection_;
    Choices choices_;
};


template
<
    typename Convert,
    typename Control
>
wxComboBox * MakeComboBox(
    wxWindow *parent,
    const Control &control,
    const std::string &nullValue = "")
{
    if constexpr (jive::IsOptional<typename Control::Selection::Type>)
    {
        return new OptionalComboBox<Control, Convert>(
            parent,
            control,
            nullValue);
    }
    else
    {
#ifndef NDEBUG
        if (!nullValue.empty())
        {
            std::cerr << "[wxpex] Warning: nullValue customization is unused"
                << std::endl;
        }
#endif

        return new ComboBox<Control, Convert>(parent, control);
    }
}


} // namespace wxpex
