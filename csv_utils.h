//
// Created by gregkwaste on 5/6/19.
//

#ifndef CRF_RESCHEDULING_CSV_UTILS_H
#define CRF_RESCHEDULING_CSV_UTILS_H

#include <vector>
#include <string>
#include <sstream>

//CSV Parser
std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
{
    std::vector<std::string>   result;
    std::string                line;
    std::getline(str,line);

    std::stringstream          lineStream(line);
    std::string                cell;

    while(std::getline(lineStream, cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return result;
}

#endif //CRF_RESCHEDULING_CSV_UTILS_H
