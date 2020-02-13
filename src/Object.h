#pragma once

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <Utils.h>

class ModelTriangle;

struct Object
{
    std::string name;
    std::vector<ModelTriangle> triangles;

    Object(std::string name, std::vector<ModelTriangle> triangles)
    : name(name)
    , triangles(triangles)
    {}

};

std::ostream& operator<<(std::ostream& os, const Object& object)
{
    os << object.name << ":" << std::endl;
    os << "[";
    for (ModelTriangle triangle : object.triangles)
    {
        os << triangle;
    }
    os << "]";
    os << std::endl;
    return os;
}

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

    ifs.close();

    return colours;
}

glm::vec3 readVertex(std::ifstream& ifs)
{
    float p0, p1, p2;
    std::string buffer;

    ifs >> buffer;
    p0 = std::stof(buffer);

    ifs >> buffer;
    p1 = std::stof(buffer);
   
    ifs >> buffer;
    p2 = std::stof(buffer);

    return glm::vec3(p0, p1, p2);
}

Object readObject(std::ifstream& ifs, std::unordered_map<std::string, Colour>& colourMap, int& totalVertices, float scaleFactor)
{
    std::string name;
    std::string colour;
    std::vector<glm::vec3> vertices;
    std::vector<ModelTriangle> triangles;

    std::string buffer;
    ifs >> name;

    ifs >> buffer;
    ifs >> colour;

    //Read Vertices
    ifs >> buffer;
    while (buffer == "v")
    {   
        vertices.push_back(readVertex(ifs) * scaleFactor);
        ifs >> buffer;
    }

    glm::vec3 v0, v1, v2;
    while (buffer == "f")
    {
        ifs >> buffer;
        buffer.erase(buffer.size() - 1); // Remove trailing '/'
        v0 = vertices[std::stoi(buffer) - totalVertices - 1];

        ifs >> buffer;
        buffer.erase(buffer.size() - 1); // Remove trailing '/'
        v1 = vertices[std::stoi(buffer) - totalVertices - 1];

        ifs >> buffer;
        buffer.erase(buffer.size() - 1); // Remove trailing '/'
        v2 = vertices[std::stoi(buffer) - totalVertices - 1];

        ModelTriangle triangle(v0, v1, v2, colourMap[colour]);

        triangles.push_back(triangle);

        ifs >> buffer;
    }

    totalVertices += vertices.size();

    return Object(name, triangles);
}

std::vector<Object> loadOBJ(const char* filepath, float scaleFactor)
{
    std::unordered_map<std::string, Colour> colourMap;
    std::vector<Object> objects;
    std::ifstream ifs(filepath, std::ifstream::in);

    std::string matFilepath;
    std::string buffer;
    ifs >> buffer;
    ifs >> buffer;

    matFilepath = "models/";
    matFilepath += buffer;

    colourMap = loadColours(matFilepath.c_str()); 
    int totalVertices = 0;
    ifs >> buffer;
    while(ifs.good())
    {
        objects.push_back(readObject(ifs, colourMap, totalVertices, scaleFactor));
    }

    ifs.close();

    return objects;
}
