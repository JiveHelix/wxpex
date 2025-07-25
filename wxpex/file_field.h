#pragma once


#include <jive/path.h>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/button.h>
#include <wx/control.h>
WXSHIM_POP_IGNORES

#include "wxpex/field.h"
#include "wxpex/labeled_widget.h"


namespace wxpex
{


struct FileDialogOptions
{
    long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
    std::string message = "";
    std::string wildcard = "";
    bool isFolder = false;

    FileDialogOptions & Style(long value_)
    {
        this->style = value_;

        return *this;
    }

    FileDialogOptions & Message(const std::string &message_)
    {
        this->message = message_;

        return *this;
    }

    FileDialogOptions & Wildcard(const std::string &wildcard_)
    {
        this->wildcard = wildcard_;

        return *this;
    }

    FileDialogOptions & IsFolder(bool isFolder_)
    {
        this->isFolder = isFolder_;

        return *this;
    }
};


std::optional<std::string> ChoosePath(
    const std::string &currentValue,
    const FileDialogOptions &options);


template<typename Control>
class FileField: public wxControl
{
public:
    static constexpr auto observerName = "FileField";

    FileField(
        wxWindow *parent,
        Control control,
        const FileDialogOptions &options = FileDialogOptions{})
        :
        wxControl(parent, wxID_ANY),
        value_(control),
        options_{options}
    {
        auto field = new Field(this, control, FixedFieldTag{});
        field->SetMinimumWidth(200);

        auto button = new wxButton(
            this,
            wxID_ANY,
            "Choose");

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(field, 1, wxRIGHT, 3);
        sizer->Add(button, 0);

        this->SetSizerAndFit(sizer.release());

        this->Bind(wxEVT_BUTTON, &FileField::OnChoose_, this);
    }

    void OnChoose_(wxCommandEvent &)
    {
        auto chosenPath = ChoosePath(this->value_.Get(), this->options_);

        if (chosenPath)
        {
            this->value_.Set(*chosenPath);
        }
    }

private:
    Control value_;
    FileDialogOptions options_;
};


template<typename Control>
struct LabeledFileField
    :
    public LabeledWidget<FileField<Control>>
{
public:
    using Base = LabeledWidget<FileField<Control>>;

    LabeledFileField(
        wxWindow *parent,
        const std::string &label,
        const Control &control,
        const FileDialogOptions &options = FileDialogOptions{})
        :
        Base(
            parent,
            label,
            new FileField<Control>(parent, control, options))
    {

    }

};


} // end namespace wxpex
