#pragma once

#include <algorithm>
#include <vector>

std::string* split(std::string line, char delim);

std::string* split(std::string line, char delim)
{
    int numberOfTokens = count(line.begin(), line.end(), delim) + 1;
    std::string *tokens = new std::string[numberOfTokens];
    int currentPosition = 0;
    int nextIndex = 0;
    for(int i=0; i<numberOfTokens ;i++) {
        nextIndex = line.find(delim, currentPosition);
        tokens[i] = line.substr(currentPosition,nextIndex-currentPosition);
        currentPosition = nextIndex + 1;
    }
    return tokens;
}

std::vector<std::string> splitV(std::string line, char delim)
{
    std::vector<std::string> tokens;
    int numberOfTokens = count(line.begin(), line.end(), delim) + 1;
    int currentPosition = 0;
    int nextIndex = 0;
    for(int i=0; i<numberOfTokens ;i++) {
        nextIndex = line.find(delim, currentPosition);
        tokens.push_back(line.substr(currentPosition,nextIndex-currentPosition));
        currentPosition = nextIndex + 1;
    }
    return tokens;
}
