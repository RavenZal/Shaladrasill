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

int Model::getVertsNumber() const
{
   return _verts_.size();
}

int Model::getFacesNumber() const
{
   return _faces_.size();
}

const vec3f& Model::getVertsFromIndex(int index) const
{
   if(_verts_.size() > index && index >= 0 )
   {
       return _verts_[index];
   }else{
       static vec3f ErrorVert;
       return ErrorVert;
   }
}

const std::vector<int>& Model::getFacesFromIndex(int index) const
{  
   if(_faces_.size() > index && index >= 0 )
   {
       return _faces_[index];
   }else{
       static std::vector<int> ErrorFacesVector;
       return ErrorFacesVector;
   }
}

const vec3f &Model::getNormalLineFrom(int faceIndex , int vertsIndex) const
{
    if(_norms_faces_.size() > faceIndex && faceIndex >=0)
    {
        return (_norms_[ _norms_faces_[faceIndex][vertsIndex]]);
    }else{
        static vec3f ErrorNormlines;
        return ErrorNormlines;
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
            std::vector<int> elementTexFaceSet;
            std::vector<int> elementNormFaceSet;
            while ( ss >> elementStr )
            {
                std::stringstream faceStream(elementStr);
                std::string item;
                std::vector<int> parsedIndices;
                while (std::getline(faceStream, item, '/'))
                {
                    if (item.empty())
                    {
                        parsedIndices.push_back(-1);
                    }else
                    {
                        parsedIndices.push_back(std::stoi(item) - 1);
                    }
                }
                elementFaceSet.push_back(parsedIndices.size() > 0 ? parsedIndices[0] : -1);
                elementTexFaceSet.push_back(parsedIndices.size() > 1 ? parsedIndices[1] : -1);
                elementNormFaceSet.push_back(parsedIndices.size() > 2 ? parsedIndices[2] : -1);
            }           
            _faces_.push_back(elementFaceSet);
            _tex_faces_.push_back(elementTexFaceSet);
            _norms_faces_.push_back(elementNormFaceSet); 
        } else if (type == "vn")
        {
            double x, y, z;
            ss >> x >> y >> z;
            vec3f elementOfvec3f(x, y, z);
            _norms_.push_back(elementOfvec3f);
        } 
    }
    std::cout << "[Debug]Model::setVarFromObj() Success" << std::endl;
    return true;   
}