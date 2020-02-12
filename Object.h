#include <fstream>
#include <iostream>


#include <unordered_map>

class Material
{

};

class Object
{

};

Colour readColour(std::ifstream& ifs)
{
    uint32_t r = 0;
    uint32_t g = 0;
    uint32_t b = 0;

    std::string buffer;
    std::string name;

    std::getline(ifs, buffer);
    name = buffer.substr(7, buffer.size());
    std::cout << "colour: " << name << std::endl;

    std::getline(ifs, buffer);
    //std::string* tokens;


    std::cout << "rgb: " << buffer << std::endl;

    std::getline(ifs, buffer);



    return Colour(name, r, g, b);
}

Material loadMaterial(const char* filepath)
{   
    std::unordered_map<std::string, Colour> palette;
    std::ifstream ifs (filepath, std::ifstream::in);

    while(ifs.good())
    {
        Colour colour = readColour(ifs); 
    }


    return Material();
}