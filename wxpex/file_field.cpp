#include <wxpex/file_field.h>


namespace wxpex
{


std::optional<std::string> ChoosePath(
    const std::string &currentValue,
    const FileDialogOptions &options)
{
    if (options.isFolder)
    {
        wxDirDialog openFolder(
            nullptr,
            wxString(options.message),
            wxString(currentValue),
            (options.style & wxFD_FILE_MUST_EXIST)
                ? wxDD_DIR_MUST_EXIST | wxDD_DEFAULT_STYLE
                : wxDD_DEFAULT_STYLE);

        if (openFolder.ShowModal() == wxID_CANCEL)
        {
            return {};
        }

        return openFolder.GetPath().utf8_string();
    }
    else
    {
        auto [directory, file] = jive::path::Split(currentValue);

        wxFileDialog openFile(
            nullptr,
            wxString(options.message),
            wxString(directory),
            wxString(file),
            wxString(options.wildcard),
            options.style);

        if (openFile.ShowModal() == wxID_CANCEL)
        {
            return {};
        }

        return openFile.GetPath().utf8_string();
    }
}


}
