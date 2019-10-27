//
// Created by gregkwaste on 5/6/19.
// TODO: Merge utilities with the ones from the Arcelik
// project and make sure to store this shit somewhere for future reference
//

#ifndef CRF_RESCHEDULING_STRING_UTILS_H
#define CRF_RESCHEDULING_STRING_UTILS_H

#include <string>
#include <vector>

void split_string_with_delim(std::string input, std::string delimiter, std::vector<std::string> &output);
std::string trim(const std::string& str);

#endif //CRF_RESCHEDULING_STRING_UTILS_H
