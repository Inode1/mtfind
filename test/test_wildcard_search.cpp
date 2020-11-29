#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <future>
#include <cassert>
#include <iterator>
#include <sstream>

#include "wildcard_search.hpp"

int main()
{
    std::vector<std::tuple<std::string,
                           std::string,
                           std::vector<std::string>>> 
                           tests {
        {"../../test/sample/input.txt", "?ad", 
                                    {"3", "5 5 bad", "6 6 mad", "7 6 had"}},
        {"../../test/sample/bible.txt", "God? s??d", 
                                    {"2", "104 81 God, said", "1246 59 God, said"}},
        {"../../test/sample/alphabet.txt", "?????????????????", 
                                    {"2", "1 1 abcdefghijklmnopq", "1 18 rstuvwxyzabcdefgh"}}

    };

    for (const auto& test: tests)
    {
        std::ifstream infile(std::get<0>(test));
        if (!infile.good())
        {
            std::cout << "Test file '" << std::get<0>(test) << "' is not exist." << std::endl;
            return 1;
        }
        std::shared_ptr<std::vector<std::string>> text = std::make_shared<std::vector<std::string>>();
        std::string line;
        while (std::getline(infile, line))
        {
            text->push_back(std::move(line));
        }
        mtfind::MTFind sequance(text, std::get<1>(test));
        sequance.Search(false);
        mtfind::MTFind parallel(text, std::get<1>(test));
        parallel.Search();
        if (parallel != sequance)
        {
            std::cout << "Test file '" << std::get<0>(test) << "' check fail." << std::endl;
            return 1;
        }

        std::stringstream in;
        std::vector<std::string> check;
        in << parallel;
        while (std::getline(in, line))
        {
            check.push_back(line);
        }
        const auto& validate = std::get<2>(test);
        if (validate.size() != check.size())
        {
            std::cout << "Test file '" << std::get<0>(test) << "' result size is not equal." << std::endl;
            return 1;            
        }
        auto diff = std::mismatch(validate.cbegin(), validate.cend(), check.cbegin());

        if (diff.first != validate.cend() || diff.second != check.cend())
        {
            std::cout << "Test file '" << std::get<0>(test) << "' result is not valid." << std::endl;
            return 1;
        }
    }
    return 0;
}