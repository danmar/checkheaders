#include "FileParser.h"
#include <iostream>
#include <cstdlib>
#include <string.h>

FileParser::FileParser(const char *fileName) :
    fileName_(strdup(fileName))
{
}

int FileParser::process(std::vector<std::string> &includePaths, std::set<std::string> &skipIncludes)
{
    std::ifstream ifile(fileName_);
    if (false == ifile.is_open())
    {
        std::cerr << "Cannot open file '" << fileName_ << "'" << std::endl;
        return 1;
    }
    std::string line;
    int section = INVALID_SECTION;
    int lineNo = 0;
    bool found = false;
    while (false == std::getline(ifile, line).eof())
    {
        ++lineNo;
        if ((true == ifile.fail()) || (true == ifile.bad()))
        {
            std::cerr << "Error when reading line " << lineNo << " from '" << fileName_ << "'" << std::endl;
            return 1;
        }
        //skip empty lines
        if (true == line.empty())
        {
            continue;
        }
        //get section type
        if (("include" == line) || ("INCLUDE" == line))
        {
            section = INCLUDE_SECTION;
            continue;
        } 
        else if (("skip" == line) || ("SKIP" == line))
        {
            section = SKIP_SECTION;
            continue;
        }
        //get includes or skips
        switch (section)
        {
            case INCLUDE_SECTION:
                includePaths.push_back(line);
                found = true;
                break;
            case SKIP_SECTION:
                skipIncludes.insert(line);
                found = true;
                break;
            default:
                std::cerr << "No skip or include section found in '" << fileName_ << "'" << std::endl;
                return 1;
        }
    }
    return (true == found)?0:1;
}

FileParser::~FileParser()
{
    free(fileName_);
}

