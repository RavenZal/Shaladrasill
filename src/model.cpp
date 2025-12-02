#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "model.h"

Model::Model(const char *filename) 
{
   setVarFromObj(filename);
}

Model::~Model() 
{
//default    
}

int Model::getVertsNumber()
{
   return _verts_.size();
}

int Model::getFacesNumber()
{
   return _faces_.size();
}

const vec3f& Model::getVertsFromIndex(int index)
{
   if(_verts_.size() > index && index >= 0 )
   {
       return _verts_[index];
   }else{
       static vec3f ErrorVert;
       return ErrorVert;
   }
}

const std::vector<int>& Model::getFacesFromIndex(int index)
{  
   if(_faces_.size() > index && index >= 0 )
   {
       return _faces_[index];
   }else{
       static std::vector<int> ErrorFacesVector;
       return ErrorFacesVector;
   }
}

bool Model::setVarFromObj(const char *filename)
{  
    std::string FILE_PATH = std::string("assets/") + filename ;
    std::ifstream objfile(FILE_PATH);
    if(!objfile.is_open())
    {
        std::cout << "[Debug]Model::setVarFromObj() Failed" << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(objfile, line))
    {
        std::stringstream ss(line);
        std::string type;
        ss >> type;
        if(type == "v")
        {
            double x, y, z;
            ss >> x >> y >> z;
            vec3f elementOfvec3f(x, y, z);
            _verts_.push_back(elementOfvec3f);
        } else if (type == "f")
        {   
            std::string elementStr;
            std::vector<int> elementFaceSet;
            while ( ss >> elementStr )
            {
                size_t positionInterval = elementStr.find('/');
                std::string elementStrFacesIndex = elementStr.substr(0,positionInterval);
                int elementIntFacesIndex = std::stoi(elementStrFacesIndex);
                elementFaceSet.clear();
                elementFaceSet.push_back(elementIntFacesIndex - 1); //index in .obj is from 1
            }
            _faces_.push_back(elementFaceSet);   
        }  
    }
    std::cout << "[Debug]Model::setVarFromObj() Success" << std::endl;
    return true;   
}