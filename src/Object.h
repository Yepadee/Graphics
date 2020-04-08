#pragma once

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <Utils.h>

#include "Interpolation.h"

using namespace glm;
class ModelTriangle;

struct Object
{
    std::string name;
    std::vector<ModelTriangle> triangles;

    Object(std::string name, std::vector<ModelTriangle> triangles)
    : name(name)
    , triangles(triangles)
    {
        //setVertexNormals();
    }

private:
    void setVertexNormals()
    {
        for (ModelTriangle& triangle1 : triangles)
        {

            for (int v = 0; v < 3; v++)
            {
                vec3 sumVertexNormal(0.0f, 0.0f, 0.0f);
                float numVerticies = 0.0f;
                
                vec3 vertex = triangle1.vertices[v];

                for (ModelTriangle triangle2 : triangles)
                {
                    if (triangleHasVertex(triangle2, vertex))
                    {
                        sumVertexNormal += triangle2.normal;
                        numVerticies ++;
                    }
                }

                triangle1.vertexNormals[v] = sumVertexNormal / numVerticies;
            }
        }
    }

    bool triangleHasVertex(const ModelTriangle& triangle, const vec3& v)
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

vec3 readVec3(std::ifstream& ifs)
{
    float p0, p1, p2;
    std::string buffer;

    ifs >> buffer;
    p0 = std::stof(buffer);

    ifs >> buffer;
    p1 = std::stof(buffer);
   
    ifs >> buffer;
    p2 = std::stof(buffer);

    return vec3(p0, p1, p2);
}

vec2 readVec2(std::ifstream& ifs)
{
    float p0, p1;
    std::string buffer;

    ifs >> buffer;
    p0 = std::stof(buffer);

    ifs >> buffer;
    p1 = std::stof(buffer);

    return vec2(p0, p1);
}

Object readObject(std::ifstream& ifs, std::unordered_map<std::string, Colour>& colourMap, int& totalVertices, float scaleFactor, vec3 displacement)
{
    std::string name;
    std::string colour;

    std::vector<vec3> vertices;
    std::vector<vec3> vertexNormals;
    std::vector<vec2> textureCoords;

    std::vector<ModelTriangle> triangles;

    std::string buffer;
    ifs >> name;

    ifs >> buffer;
    ifs >> colour;

    //Read Vertices
    ifs >> buffer;
    while (buffer == "v")
    {   
        vertices.push_back(readVec3(ifs) * scaleFactor);
        ifs >> buffer;
    }

    while (buffer == "vn")
    {   
        vertexNormals.push_back(readVec3(ifs) * scaleFactor);
        ifs >> buffer;
    }

    while (buffer == "vt")
    {
        textureCoords.push_back(readVec2(ifs));
        ifs >> buffer;
    }

    bool hasVertexNormals = vertexNormals.size() > 0;

    vec3 v0, v1, v2;
    vec3 n0, n1, n2;
    int t0, t1, t2;
    int f0, f1, f2;

    std::vector<std::string> tokens;

    while (buffer == "f")
    {
        bool hasTexture = false;
        ifs >> buffer;
        tokens = splitV(buffer, '/');
        if (tokens[1] != "") hasTexture = true;


        f0 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) t0 = std::stoi(tokens[1]) - totalVertices - 1;

        ifs >> buffer;
        tokens = splitV(buffer, '/');
        f1 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) t1 = std::stoi(tokens[1]) - totalVertices - 1;

        ifs >> buffer;
        tokens = splitV(buffer, '/');
        f2 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) t2 = std::stoi(tokens[1]) - totalVertices - 1;

        v0 = vertices[f0] + displacement;
        v1 = vertices[f1] + displacement;
        v2 = vertices[f2] + displacement;

        if (hasVertexNormals)
        {
            n0 = vertexNormals[f0];
            n1 = vertexNormals[f1];
            n2 = vertexNormals[f2];

            ModelTriangle triangle(v0, v1, v2,
                                   n0, n1, n2,
                                   colourMap[colour]);
            triangles.push_back(triangle);
        }
        else
        {
            ModelTriangle triangle(v0, v1, v2,
                                   colourMap[colour]);
            triangles.push_back(triangle);
        }

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
std::vector<Object> loadOBJ(const char* filepath, float scaleFactor, vec3 displacement)
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
        objects.push_back(readObject(ifs, colourMap, totalVertices, scaleFactor, displacement));
    }

    ifs.close();

    return objects;
}
