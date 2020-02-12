#include <fstream>
#include <iostream>
#include <unordered_map>

std::string* split(std::string line, char delim);

class ModelTriangle;

class Object
{
private:
    std::string name;
    std::string colour;
    std::vector<ModelTriangle> triangles;

public:
    Object(std::string name, std::string colour, std::vector<ModelTriangle> triangles)
    : name(name)
    , colour(colour)
    , triangles(triangles)
    {}
};


class ObjectCollection
{
private:
    std::unordered_map<std::string, Colour> colours;
    std::vector<Object> objects;

public:
    ObjectCollection(std::unordered_map<std::string, Colour> colours, std::vector<Object> objects)
    : colours(colours)
    , objects(objects)
    {}
    
    std::unordered_map<std::string, Colour> getColours()
    {
        return colours;
    }

    std::vector<Object> getObjects()
    {
        return objects;
    }

};


Colour readColour(std::ifstream& ifs)
{
    Colour colour;
    std::string buffer;
    float rf, gf, bf;

    std::getline(ifs, buffer);
    colour.name = buffer.substr(7, buffer.size());

    std::getline(ifs, buffer);
    std::string* tokens = split(buffer, ' ');

    rf = std::stof(tokens[1]);
    gf = std::stof(tokens[2]);
    bf = std::stof(tokens[3]);

    colour.red   = (int) (rf * 255.0f);
    colour.green = (int) (gf * 255.0f);
    colour.blue  = (int) (bf * 255.0f);

    std::getline(ifs, buffer);

    return colour;
}

std::unordered_map<std::string, Colour> loadColours(const char* filepath)
{   
    std::unordered_map<std::string, Colour> colours;
    std::ifstream ifs(filepath, std::ifstream::in);

    while(ifs.good())
    {
        Colour colour = readColour(ifs);
        colours[colour.name] = colour;
        
    }

    return colours;
}

Object readObject()
{

}

ObjectCollection loadOBJ(const char* filepath)
{
    std::unordered_map<std::string, Colour> colours;
    std::vector<Object> objects;
    std::ifstream ifs(filepath, std::ifstream::in);
    
    std::string matFilepath;
    matFilepath << ifs;

    colours = loadColours(matFilepath.c_str()); 
    
    return ObjectCollection(colours, objects);
}
