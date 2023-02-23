#pragma once


#include <jive/path.h>
#include <pex/value.h>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/button.h>
#include <wx/control.h>
WXSHIM_POP_IGNORES

#include "wxpex/field.h"


namespace wxpex
{


struct FileDialogOptions
{
    long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
    std::string message = "";
    std::string directory = "";
    std::string file = "";
    std::string wildcard = "";
};


template<typename Control>
class FileField: public wxControl
{
public:
    static constexpr auto observerName = "FileField";

    FileField(
        wxWindow *parent,
        Control value,
        const FileDialogOptions &options = FileDialogOptions{})
        :
        wxControl(parent, wxID_ANY),
        value_{this, value},
        options_{options}
    {
        this->OnValue_(this->value_.Get());
        this->value_.Connect(&FileField<Control>::OnValue_);

        auto field = new wxpex::Field(this, value);
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
        wxFileDialog openFile(
            nullptr,
            wxString(this->options_.message),
            wxString(this->options_.directory),
            wxString(this->options_.file),
            wxString(this->options_.wildcard),
            this->options_.style);

        if (openFile.ShowModal() == wxID_CANCEL)
        {
            return;
        }

        this->value_.Set(openFile.GetPath());
    }

    void OnValue_(const std::string &value)
    {
        if (value.empty())
        {
            return;
        }

        auto split = jive::path::Split(value);
        this->options_.directory = split.first;
        this->options_.file = split.second;
    }

private:
    using Value = pex::Terminus<FileField<Control>, Control>;
    Value value_;

    FileDialogOptions options_;
};



} // end namespace wxpex
