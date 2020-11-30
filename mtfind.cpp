#include <iostream>
#include <fstream>

#include "wildcard_search.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Specified file and pattern to search." << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.good())
    {
        std::cout << "Test file '" << argv[1] << "' is not exist." << std::endl;
        return 1;
    }

    std::string pattern = argv[2];
    if (pattern.empty())
    {
        std::cout << "Pattern is empty." << std::endl;
        return 2;
    }

    std::shared_ptr<std::vector<std::string>> text = std::make_shared<std::vector<std::string>>();
    std::string line;
    while (std::getline(infile, line))
    {
        text->push_back(std::move(line));
    }
    mtfind::MTFind parallel(text, pattern);
    parallel.Search();

    std::cout << parallel;
    return 0;
}