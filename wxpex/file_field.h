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
    bool isFolder = false;
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
        if (this->options_.isFolder)
        {
            wxDirDialog openFolder(
                nullptr,
                wxString(this->options_.message),
                wxString(this->value_.Get()),
                (this->options_.style & wxFD_FILE_MUST_EXIST)
                    ? wxDD_DIR_MUST_EXIST | wxDD_DEFAULT_STYLE
                    : wxDD_DEFAULT_STYLE);

            if (openFolder.ShowModal() == wxID_CANCEL)
            {
                return;
            }

            this->value_.Set(openFolder.GetPath());
        }
        else
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
    }

private:
    Control value_;
    FileDialogOptions options_;
};



} // end namespace wxpex
