#include <iostream>
#include "common.h"
#include "ini_parser.h"
#include "log.h"


int main(int argc, char* argv[])
{
    if (IniParser::getInstance()->FnReadIniFile() == false)
    {
        std::exit(EXIT_FAILURE);
    }
    Logger::getInstance();
    Common::getInstance()->FnLogExecutableInfo(argv[0]);

    return 0;
}