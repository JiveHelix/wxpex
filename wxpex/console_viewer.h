#pragma once

#include <wxpex/async.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include <wx/tokenzr.h>

WXSHIM_PUSH_IGNORES
#include <wx/richtext/richtextctrl.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


template<typename T>
struct ConsoleViewerFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::clear, "clear"),
        fields::Field(&T::message, "message"));
};

template<template<typename> typename T>
struct ConsoleViewerTemplate
{
    T<pex::MakeSignal> clear;
    T<MakeAsync<std::string>> message;
};


using ConsoleViewerGroup =
    pex::Group<ConsoleViewerFields, ConsoleViewerTemplate>;

using ConsoleViewerModel = typename ConsoleViewerGroup::Model;
using ConsoleViewerControl = typename ConsoleViewerGroup::DefaultControl;
using ConsoleMessageControl = decltype(ConsoleViewerControl::message);


struct AnsiState
{
    bool bold = false;
    bool underline = false;
    std::optional<wxColour> fg;
    std::optional<wxColour> bg;
};


static std::map<int, wxColour> SgrBasicColor =
{
    {30, wxColour(0,0,0)},
    {31, wxColour(205,49,49)},
    {32, wxColour(13,188,121)},
    {33, wxColour(229,229,16)},
    {34, wxColour(36,114,200)},
    {35, wxColour(188,63,188)},
    {36, wxColour(17,168,205)},
    {37, wxColour(229,229,229)},
    {90, wxColour(102,102,102)},
    {91, wxColour(241,76,76)},
    {92, wxColour(35,209,139)},
    {93, wxColour(245,245,67)},
    {94, wxColour(59,142,234)},
    {95, wxColour(214,112,214)},
    {96, wxColour(41,184,219)},
    {97, wxColour(255,255,255)},
};


static void ApplySgrCode(AnsiState &st, int code)
{
    if (code == 0) { st = AnsiState{}; return; }
    if (code == 1) st.bold = true;
    if (code == 4) st.underline = true;
    if (code == 22) st.bold = false;
    if (code == 24) st.underline = false;

    if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97))
    {
        st.fg = SgrBasicColor[code];
    }
    if (code == 39) st.fg.reset();

    if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107))
    {
        int fgCode = (code >= 100 ? (code - 100 + 90) : (code - 40 + 30));
        auto it = SgrBasicColor.find(fgCode);
        if (it != SgrBasicColor.end()) st.bg = it->second;
    }
    if (code == 49) st.bg.reset();
}


struct StyledChunk
{
    wxString text;
    AnsiState style;
};


static std::vector<StyledChunk> ParseAnsiRuns(const wxString &chunk)
{
    std::vector<StyledChunk> out;
    AnsiState cur;
    wxString buffer;

    const wxUniChar escape(27);
    enum class State { Text, EscSeen, Csi };
    State s = State::Text;
    wxString csiBuf;

    auto Flush = [&]()
    {
        if (!buffer.empty())
        {
            out.push_back({buffer, cur});
            buffer.clear();
        }
    };

    for (size_t i = 0; i < chunk.size(); ++i)
    {
        wxUniChar ch = chunk[i];

        if (s == State::Text)
        {
            if (ch == escape)
            {
                s = State::EscSeen;
            }
            else
            {
                buffer.append(ch);
            }
        }
        else if (s == State::EscSeen)
        {
            if (ch == '[')
            {
                s = State::Csi;
                csiBuf.clear();
            }
            else
            {
                buffer.append(escape);
                buffer.append(ch);
                s = State::Text;
            }
        }
        else if (s == State::Csi)
        {
            if ((ch >= '0' && ch <= '9') || ch == ';')
            {
                csiBuf.append(ch);
            }
            else
            {
                if (ch == 'm')
                {
                    Flush();

                    if (csiBuf.empty())
                    {
                        ApplySgrCode(cur, 0);
                    }
                    else
                    {
                        wxStringTokenizer tok(csiBuf, ";");

                        while (tok.HasMoreTokens())
                        {
                            long v = 0;

                            if (tok.GetNextToken().ToLong(&v))
                            {
                                ApplySgrCode(cur, static_cast<int>(v));
                            }
                        }
                    }

                    s = State::Text;
                }
                else
                {
                    // ignore unhandled
                    s = State::Text;
                }
            }
        }
    }

    Flush();
    return out;
}


class ConsoleViewer : public wxRichTextCtrl
{
public:
    ConsoleViewer(wxWindow *parent, const ConsoleViewerControl &control)
        :
        wxRichTextCtrl(
            parent,
            wxID_ANY,
            "",
            wxDefaultPosition,
            wxDefaultSize,
            wxRE_MULTILINE | wxRE_READONLY | wxBORDER_THEME),

        clearEndpoint_(this, control.clear, &ConsoleViewer::OnClear_),
        messageEndpoint_(this, control.message, &ConsoleViewer::OnMessage_)
    {
        wxFont base = wxFontInfo(11).Family(wxFONTFAMILY_MODERN);
        this->SetFont(base);
        this->SetBackgroundColour(*wxBLACK);
    }

private:
    void OnClear_()
    {
        this->Clear();
    }

    void OnMessage_(const std::string &message)
    {
        auto runs = ParseAnsiRuns(message);

        this->Freeze();
        this->BeginSuppressUndo();

        for (auto &run : runs)
        {
            wxTextAttrEx a;

            a.SetFontWeight(
                run.style.bold
                ? wxFONTWEIGHT_BOLD
                : wxFONTWEIGHT_NORMAL);

            a.SetFontUnderlined(run.style.underline);
            a.SetTextColour(run.style.fg ? *run.style.fg : *wxWHITE);
            a.SetBackgroundColour(run.style.bg ? *run.style.bg : *wxBLACK);

            this->BeginStyle(a);
            this->WriteText(run.text);
            this->EndStyle();
        }

        this->EndSuppressUndo();
        this->Thaw();
        this->ShowPosition(this->GetLastPosition());
    }

    using ClearEndpoint =
        pex::Endpoint<ConsoleViewer, decltype(ConsoleViewerControl::clear)>;

    using MessageEndpoint =
        pex::Endpoint<ConsoleViewer, decltype(ConsoleViewerControl::message)>;

    ClearEndpoint clearEndpoint_;
    MessageEndpoint messageEndpoint_;
};


#if 0


class TeeBuffer : public std::streambuf
{
public:
    static constexpr long bufferSize = 4096;

    TeeBuffer(std::streambuf *sink, const ConsoleMessageControl &control)
        :
        sink_(sink),
        control_(control.GetWorkerControl())
    {
        this->setp(this->buffer_, this->buffer_ + bufferSize);
    }

    ~TeeBuffer() override
    {
        this->Sync();
    }

protected:
    int overflow(int character) override
    {
        if (character == traits_type::eof())
        {
            if (this->Sync() == 0)
            {
                return traits_type::not_eof(character);
            }
            else
            {
                return traits_type::eof();
            }
        }

        if (this->pptr() == this->epptr())
        {
            // The put area is full.
            if (this->Sync() != 0)
            {
                // Sync failed.
                return traits_type::eof();
            }
        }

        *this->pptr() = static_cast<char>(character);
        this->pbump(1);

        if (character == '\n')
        {
            if (this->Sync() != 0)
            {
                return traits_type::eof();
            }
        }

        return character;
    }

    int sync() override
    {
        return this->Sync();
    }

private:
    int GetCount() const
    {
        return static_cast<int>(this->pptr() - this->pbase());
    }

    int Sync()
    {
        if (this->GetCount() > 0)
        {
            std::string asString(this->pbase(), this->pptr());

            if (this->sink_ != nullptr)
            {
                this->sink_->sputn(asString.data(), asString.size());
                this->sink_->pubsync();
            }

            this->control_.Set(asString);

            this->setp(buffer_, buffer_ + bufferSize);
        }

        return 0;
    }

    std::streambuf *sink_ = nullptr;
    ConsoleMessageControl control_;
    char buffer_[static_cast<size_t>(bufferSize)];
};


#endif


} // end namespace wxpex
