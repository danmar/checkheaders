#ifndef fileparserH
#define fileparserH

#include <vector>
#include <set>
#include <string>
#include <fstream>

class FileParser
{
public:
    explicit FileParser(const char *fileName);
    int process(std::vector<std::string> &includePaths, std::set<std::string> &skipIncludes);
    ~FileParser();
private:
    enum {INVALID_SECTION, INCLUDE_SECTION, SKIP_SECTION};
    char *fileName_;
};

#endif
