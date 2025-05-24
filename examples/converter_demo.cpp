#include "wxpex/converter.h"
#include <iostream>
#include <cstring>


int main(int argCount, char **args)
{
    if (argCount != 2)
    {
        std::cerr << "Expected integer argument" << std::endl;

        return 0;
    }

    int value = std::stoi(args[1]);

    using Converter = pex::Converter<int, wxpex::PreciseTraits<0>>;

    std::cout << "format: " << Converter::ConvertToString::format << std::endl;
    std::cout << "Traits::width: " << wxpex::PreciseTraits<0>::width << std::endl;

    std::cout << "Traits::precision: " << wxpex::PreciseTraits<0>::precision << std::endl;

    std::cout << value << ": " << Converter::ToString(value) << std::endl;

    printf("%0.0d\n", value);

    return 0;
}
