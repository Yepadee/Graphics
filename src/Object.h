#pragma once

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <Utils.h>

#include "Interpolation.h"

class ModelTriangle;

struct Object
{
    std::string name;
    std::vector<ModelTriangle> triangles;

    Object(std::string name, std::vector<ModelTriangle> triangles)
    : name(name)
    , triangles(triangles)
    {
        setVertexNormals();
    }

private:
    void setVertexNormals()
    {
        std::cout << "before: " << std::endl;

        for (ModelTriangle& triangle1 : triangles)
        {

            for (int v = 0; v < 3; v++)
            {
                glm::vec3 sumVertexNormal = {0.0f, 0.0f, 0.0f};
                float numVerticies = 0.0f;
                
                glm::vec3 vertex = triangle1.vertices[v];

                for (ModelTriangle triangle2 : triangles)
                {
                    if (triangleHasVertex(triangle2, vertex))
                    {
                        sumVertexNormal += triangle2.normal;
                        numVerticies ++;
                    }
                }

                triangle1.vertexNormals[v] = sumVertexNormal / numVerticies;
                std::cout << triangle1.vertexNormals[v] << std::endl;
            }
            
            std::cout << std::endl;
            std::cout << std::endl;
        }

        std::cout << "after: " << std::endl;
        for (ModelTriangle t : triangles)
        {
            for (int i = 0; i < 3; ++ i)
            {
                std::cout << t.vertexNormals[i] << ", ";
            }
            std::cout << std::endl;
            std::cout << std::endl;
        }

    }

    bool triangleHasVertex(const ModelTriangle& triangle, const glm::vec3& v)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (triangle.vertices[i] == v) return true;
        }
        return false;
    }

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



void setVertexNormals(Object& object)
{

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
    Object object(name, triangles);

    return object;
}

/**
 * Load a .obj file into a list of 3D objects.
 *
 * @param filepath The location of the .obj file.
 * @param scaleFactor A value all the vertices describing the objects will be scaled by.
 * @return a vector containing all the objects within the loaded file.
 */
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
