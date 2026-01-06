#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

extern const std::vector<
    std::pair <
        std::string, std::vector <
            std::pair <
                std::string, std::map <
                    std::string, std::vector <
                        std::string
                    >
                >
            >
        >
    >
> browsers;

extern const std::string bootVersion;
extern const std::string sineVersion;
extern const bool isCosine;