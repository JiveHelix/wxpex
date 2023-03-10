#pragma once


#include <jive/path.h>

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
    std::string wildcard = "";
};


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
        value_(this, control),
        options_{options}
    {
        auto field = new wxpex::Field(this, control);
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
        auto [directory, file] = jive::path::Split(this->value_.Get());

        wxFileDialog openFile(
            nullptr,
            wxString(this->options_.message),
            wxString(directory),
            wxString(file),
            wxString(this->options_.wildcard),
            this->options_.style);

        if (openFile.ShowModal() == wxID_CANCEL)
        {
            return;
        }

        this->value_.Set(openFile.GetPath());
    }

private:
    using Value = pex::Terminus<FileField, Control>;
    Value value_;
    FileDialogOptions options_;
};



} // end namespace wxpex
