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


struct DirectoryDialogOptions
{
    long style = wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST;
    std::string message = "";
};


template<typename Control>
class DirectoryField: public wxControl
{
public:
    static constexpr auto observerName = "DirectoryField";

    DirectoryField(
        wxWindow *parent,
        Control control,
        const DirectoryDialogOptions &options = DirectoryDialogOptions{})
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

        this->Bind(wxEVT_BUTTON, &DirectoryField::OnChoose_, this);
    }

    void OnChoose_(wxCommandEvent &)
    {
        directory = this->value_.Get();

        wxDirDialog directoryDialog(
            nullptr,
            wxString(this->options_.message),
            wxString(directory),
            this->options_.style);

        if (directoryDialog.ShowModal() == wxID_CANCEL)
        {
            return;
        }

        this->value_.Set(directoryDialog.GetPath());
    }

private:
    Control value_;
    DirectoryDialogOptions options_;
};



} // end namespace wxpex
