#pragma once
#include <vector>
#include "geometry.h"

#define vec3f vec<3> 
class Model {
private:
    std::vector<vec3f> _verts_; //store all verts
    std::vector<std::vector<int>> _faces_; //store all faces

public:
    Model(const char *filename);
    ~Model(); //default

    //interfaces
    int getVertsNumber(); //maybe useless, get the full number of verts
    int getFacesNumber(); //get the full number of verts
    const vec3f& getVertsFromIndex(int index); //get the verts information
    const std::vector<int>& getFacesFromIndex(int index); //get the faces information

protected:
    bool setVarFromObj(const char *filename); //support function in create function
    
};