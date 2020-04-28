#pragma once

#include <glm/gtc/type_ptr.hpp>

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

    Image texture;
    bool hasTexture;

    Object(std::string name, std::vector<ModelTriangle> triangles)
    : name(name)
    , triangles(triangles)
    {
        hasTexture = false;
    }

    Object(std::string name, std::vector<ModelTriangle> triangles, Image texture)
    : name(name)
    , triangles(triangles)
    , texture(texture)
    {
        hasTexture = true;
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

    // Read colour name
    ifs >> buffer;
    colour.name = buffer;

    // Read light type
    ifs >> buffer;

    // Read red light
    ifs >> buffer;
    rf = std::stof(buffer);

    // Read green light
    ifs >> buffer;
    gf = std::stof(buffer);

    // Read blue light
    ifs >> buffer;
    bf = std::stof(buffer);

    colour.red   = (int) (rf * 255.0f);
    colour.green = (int) (gf * 255.0f);
    colour.blue  = (int) (bf * 255.0f);

    return colour;
}

std::unordered_map<std::string, Colour> loadColours(const char* filepath)
{   
    std::unordered_map<std::string, Colour> colours;
    std::ifstream ifs(filepath, std::ifstream::in);

    // Skip texture info

    std::string buffer;
    ifs >> buffer;
    while(buffer != "newmtl")
    {
        ifs >> buffer;
    }

    while(buffer == "newmtl" && ifs.good())
    {
        Colour colour = readColour(ifs);
        colours[colour.name] = colour;
        ifs >> buffer;
    }

    ifs.close();

    return colours;
}

Image readImage(std::ifstream& ifs)
{
    std::string textureFileName;
    ifs >> textureFileName;

    std::string textureFilepath = "models/";
    textureFilepath += textureFileName;

    return loadPPM(textureFilepath.c_str());
}

void loadMaterials(const char* filepath, std::unordered_map<std::string, Colour>& colourMap,
                                         std::unordered_map<std::string, Image>& textureMap)
{   
    std::ifstream ifs(filepath, std::ifstream::in);
    std::string buffer;
    ifs >> buffer;

    while(buffer == "newmtl" && ifs.good())
    {
        Colour colour = readColour(ifs);
        colourMap[colour.name] = colour;
        ifs >> buffer;

        if (buffer == "map_Kd")
        {
            Image image = readImage(ifs);
            textureMap[colour.name] = image;
            ifs >> buffer;
        }
    }

    ifs.close();
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

Object readObject(std::ifstream& ifs, std::unordered_map<std::string, Colour>& colourMap, std::unordered_map<std::string, Image>& textureMap, int& totalVertices, int& totalTextureVertices, float scaleFactor, vec3 displacement)
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

    vec3 v0, v1, v2;
    vec2 t0, t1, t2;
    vec3 n0, n1, n2;

    int fv0, fv1, fv2;
    int ft0, ft1, ft2;
    int fn0, fn1, fn2;

    std::vector<std::string> tokens;

    while (buffer == "f")
    {
        bool hasTexture = false;
        bool hasNormals = false;
        ifs >> buffer;
        tokens = splitV(buffer, '/');
        if (tokens.size() > 1 && textureMap.size() > 0) hasTexture = tokens[1] != "";
        if (tokens.size() > 2) hasNormals = true;


        fv0 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) ft0 = std::stoi(tokens[1]) - totalTextureVertices - 1;
        if (hasNormals) fn0 = std::stoi(tokens[2]) - totalVertices - 1;

        ifs >> buffer;
        tokens = splitV(buffer, '/');
        fv1 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) ft1 = std::stoi(tokens[1]) - totalTextureVertices - 1;
        if (hasNormals) fn1 = std::stoi(tokens[2]) - totalVertices - 1;

        ifs >> buffer;
        tokens = splitV(buffer, '/');
        fv2 = std::stoi(tokens[0]) - totalVertices - 1;
        if (hasTexture) ft2 = std::stoi(tokens[1]) - totalTextureVertices - 1;
        if (hasNormals) fn2 = std::stoi(tokens[2]) - totalVertices - 1;

        v0 = vertices[fv0] + displacement;
        v1 = vertices[fv1] + displacement;
        v2 = vertices[fv2] + displacement;


        bool isMirror = colourMap[colour].name == "Yellow";
        bool isGlass = colourMap[colour].name == "Red";
        ModelTriangle triangle(v0, v1, v2, colourMap[colour], isMirror, isGlass); 

        if (hasNormals)
        {
            n0 = vertexNormals[fn0];
            n1 = vertexNormals[fn1];
            n2 = vertexNormals[fn2];
            triangle.setVertexNormals(n0, n1, n2);
        }

        if (hasTexture)
        {
            
            t0 = textureCoords[ft0];
            t1 = textureCoords[ft1];
            t2 = textureCoords[ft2];
            triangle.setTexturePoints(t0, t1, t2);
        }

        triangles.push_back(triangle);

        ifs >> buffer;
    }

    totalVertices += vertices.size();
    totalTextureVertices += textureCoords.size();

    if (textureMap.count(colour))
    {
        Image texture = textureMap[colour];
        Object object(name, triangles, texture);
        return object;
    }
    else
    {
        Object object(name, triangles);
        return object;
    }
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
    std::unordered_map<std::string, Image> textureMap;

    std::vector<Object> objects;
    std::ifstream ifs(filepath, std::ifstream::in);

    std::string matFilepath;
    std::string buffer;
    ifs >> buffer;
    ifs >> buffer;

    matFilepath = "models/";
    matFilepath += buffer;

    loadMaterials(matFilepath.c_str(), colourMap, textureMap);

    int totalVertices = 0;
    int totalTextureVertices = 0;
    ifs >> buffer;
    while(ifs.good())
    {
        objects.push_back(readObject(ifs, colourMap, textureMap, totalVertices, totalTextureVertices, scaleFactor, displacement));
    }

    ifs.close();

    return objects;
}

void rotateObjectZ(Object& object, const float angle, const vec3& axis)
{
    float values[9] = {
     cos(angle), -sin(angle), 0.0f,
     sin(angle), cos(angle) , 0.0f,
     0.0f      , 0.0f       , 1.0f
    };

    mat3x3 rotation = transpose(make_mat3(values));

    for (ModelTriangle& modelTriangle : object.triangles)
    {
        for (int i = 0; i < 3; ++i)
        {
            modelTriangle.vertices[i] = rotation * (modelTriangle.vertices[i] - axis) + axis;
        }
    }
}

void translateObject(Object& object, const vec3& translation)
{
    for (ModelTriangle& modelTriangle : object.triangles)
    {
        for (int i = 0; i < 3; ++i)
        {
            modelTriangle.vertices[i] += translation;
        }
    }    
}