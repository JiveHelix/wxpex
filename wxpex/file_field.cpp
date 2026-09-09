#include <wxpex/file_field.h>

WXSHIM_PUSH_IGNORES
#include <wx/filename.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


std::optional<std::string> ChoosePath(
    const std::string &directory,
    const std::string &fileName,
    const FileDialogOptions &options)
{
    auto defaultDirectory =
        directory.empty()
        ? wxFileName::GetHomeDir()
        : wxString(directory);

    if (options.isFolder)
    {
        wxDirDialog openFolder(
            nullptr,
            wxString(options.message),
            defaultDirectory,
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
        wxFileDialog openFile(
            nullptr,
            wxString(options.message),
            defaultDirectory,
            wxString(fileName),
            wxString(options.wildcard),
            options.style);

        if (openFile.ShowModal() == wxID_CANCEL)
        {
            return {};
        }

        return openFile.GetPath().utf8_string();
    }
}


std::optional<std::string> ChoosePath(
    const std::string &currentValue,
    const FileDialogOptions &options)
{
    if (options.isFolder)
    {
        return ChoosePath(currentValue, "", options);
    }
    else
    {
        auto [directory, file] = jive::path::Split(currentValue);

        return ChoosePath(directory, file, options);
    }
}


}
