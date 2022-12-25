#ifndef GRAPHICSDATA_H_INCLUDED
#define GRAPHICSDATA_H_INCLUDED
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#define M_PI 3.14159265358979323846


using glm::vec3;
using glm::mat4;
using std::unordered_map;
using std::string;
using std::cout;
using std::pair;
using std::ifstream;
using std::stringstream;

typedef vec3 (*CRTPNT)(double, vec3, double);

enum GL_DATA{
    SHADER,
    BUFFER,
    PROGRAMM
};

enum GL_PLANE{
    XZ,
    XY,
    ZY
};

class GraphicsData{
private:
    static mat4 projection;
    static mat4 modelView;
    static unordered_map<string,pair<unsigned int,GL_DATA> > gData;
    static vec3 CreatePointXZ(double angle, vec3 position, double radius)
    {
        vec3 result;
        result.x = (float)(position.x + radius * cos(angle));
        result.y =  position.y + 0.0f;
        result.z = (float)(position.z + radius * -sin(angle));
        return result;
    }
    static vec3 CreatePointXY(double angle, vec3 position, double radius)
    {
        vec3 result;
        result.x = (float)(position.x + radius * cos(angle));
        result.y = (float)(position.y + radius * sin(angle));
        result.z = position.z + 0.0f;
        return result;
    }
    static vec3 CreatePointZY(double angle, vec3 position, double radius)
    {
        vec3 result;
        result.x = position.x + 0.0f;
        result.y = (float)(position.y + radius * sin(angle));
        result.z = (float)(position.z + radius * -cos(angle));
        return result;
    }
public:
    static unsigned int LoadShader(string path, unsigned int type ,string key){
        if(key.empty())
            return 0;
        auto res = gData.find(key);
        if(res != gData.end())
            return (*res).second.first;
        ifstream file;
        stringstream sstream;
        file.open(path);
        if(!file)
        {
            cout<< "File not opened";
            return 0;
        }
        sstream << file.rdbuf();
        auto srcShader = sstream.str();
        auto src = srcShader.c_str();
        auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        int succes;
        glGetShaderiv(shader,GL_COMPILE_STATUS,&succes);
        if(!succes){
            char infoLog[1024];
            glGetShaderInfoLog(shader,1024,NULL,infoLog);
            cout<< "Error on file " << path <<"\n Log:\n" << infoLog;
            glDeleteShader(shader);
            shader = 0;
        }
        else
            gData[key] = std::make_pair(shader,SHADER);
        return shader;
    }
    static unordered_map<string,pair<unsigned int,GL_DATA> >& Data()
    {
        return gData;
    }
    static void CreateFace(float vertices[],unsigned int indices[],unsigned int vWrite,float radius,
                           vec3 pos, GL_PLANE plane = XZ, bool isCCW = true, bool calcNormal = true)
    {
        static unsigned int iRead = 0;
        static unsigned int iWrite = 0;
        CRTPNT point = plane == XZ ? &GraphicsData::CreatePointXZ
                    : plane == XY ? &GraphicsData::CreatePointXY
                    : &GraphicsData::CreatePointZY;
        float initAngle = isCCW ?  -M_PI / 4 : 5 * M_PI / 4;
        float sectorAngle = isCCW ? 2 * M_PI / 4 : -2 * M_PI / 4;
        unsigned int normalStride = calcNormal ? 3 : 0;
        for(unsigned int i = 0; i < 4; ++i, initAngle += sectorAngle)
        {
            unsigned int offset = i * (3 + normalStride);
            vec3 coords = point(initAngle, pos, radius);
            vertices[vWrite + offset] = coords.x;
            vertices[vWrite + offset + 1] = coords.y;
            vertices[vWrite + offset + 2] = coords.z;
            if(i < 3)
                indices[iRead++] = iWrite + i;
            else
            {
                indices[iRead++] = iWrite + i - 1;
                indices[iRead++] = iWrite + i;
                indices[iRead++] = iWrite;
            }
        }
        iWrite+=4;
        if(calcNormal)
        {
            vec3 normal = glm::cross(vec3(vertices[vWrite],vertices[vWrite + 1],vertices[vWrite + 2]),
                                     vec3(vertices[vWrite + 6],vertices[vWrite + 7],vertices[vWrite + 8]));
            for(unsigned int i = 0; i < 4; ++i, normalStride+=6)
            {
                vertices[vWrite + normalStride] = normal.x;
                vertices[vWrite + normalStride + 1] = normal.y;
                vertices[vWrite + normalStride + 2] = normal.z;
            }
        }
    }
};
#endif // GRAPHICSDATA_H_INCLUDED
