//
// Created by gregkwaste on 5/6/19.
//


#include "string_utils.h"

void split_string_with_delim(std::string input, std::string delimiter, std::vector<std::string> &output){
    ulong pos;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        std::string token = input.substr(0, pos);
        output.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    output.push_back(input);
}

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}