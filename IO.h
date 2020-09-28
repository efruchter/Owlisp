#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <functional>
#include <sstream>
#include <fstream>

#include "Containers.h"

Return<std::string, std::string> ReadFileIntoString( const std::string& FileName ) {
    std::ifstream inFile{ FileName };

    if ( inFile.fail() ) {
        return { "", "Error: File open failed." };
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();

    return { strStream.str() };
}
