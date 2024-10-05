#include "graphics.h"


namespace wxpex
{


std::vector<InterpolationQuality> GetInterpolationQualities()
{
    return {
        InterpolationQuality::DEFAULT,
        InterpolationQuality::none,
        InterpolationQuality::fast,
        InterpolationQuality::good,
        InterpolationQuality::best};
}


std::string InterpolationQualityConverter::ToString(
    InterpolationQuality interpolationQuality)
{
    switch (interpolationQuality)
    {
        case (InterpolationQuality::DEFAULT):
            return "DEFAULT";

        case (InterpolationQuality::none):
            return "none";

        case (InterpolationQuality::fast):
            return "fast";

        case (InterpolationQuality::good):
            return "good";

        case (InterpolationQuality::best):
            return "best";

        default:
            throw std::logic_error("Unknown InterpolationQuality");
    }
}


std::ostream & operator<<(std::ostream &output, InterpolationQuality value)
{
    return output << InterpolationQualityConverter::ToString(value);
}


std::vector<CompositionMode> CompositionModeChoices::GetChoices()
{
    return {
        // CompositionMode::invalid,
        // CompositionMode::clear,
        CompositionMode::source,
        CompositionMode::over,
        CompositionMode::in,
        CompositionMode::out,
        CompositionMode::atop,
        CompositionMode::dest,
        CompositionMode::destOver,
        CompositionMode::destIn,
        CompositionMode::destOut,
        CompositionMode::destAtop,
        CompositionMode::XOR,
        CompositionMode::add,
        CompositionMode::diff};
}


std::string CompositionModeConverter::ToString(CompositionMode compositionMode)
{
    switch (compositionMode)
    {
        case (CompositionMode::invalid):
            return "invalid";

        case (CompositionMode::clear):
            return "clear";

        case (CompositionMode::source):
            return "source";

        case (CompositionMode::over):
            return "over";

        case (CompositionMode::in):
            return "in";

        case (CompositionMode::out):
            return "out";

        case (CompositionMode::atop):
            return "atop";

        case (CompositionMode::dest):
            return "dest";

        case (CompositionMode::destOver):
            return "dest over";

        case (CompositionMode::destIn):
            return "dest in";

        case (CompositionMode::destOut):
            return "dest out";

        case (CompositionMode::destAtop):
            return "dest atop";

        case (CompositionMode::XOR):
            return "XOR";

        case (CompositionMode::add):
            return "add";

        case (CompositionMode::diff):
            return "diff";

        default:
            throw std::logic_error("Unknown CompositionMode");
    }
}


std::ostream & operator<<(std::ostream &output, CompositionMode value)
{
    return output << CompositionModeConverter::ToString(value);
}


} // end namespace wxpex
