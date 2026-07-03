#pragma once
#include <vector>
#include "geometry.h"
#include "tgaimage.h"

#define vec3f vec<3> 
class Model {
private:
    std::vector<vec3f> _verts_; //store all verts
    std::vector<std::vector<int>> _faces_; //store all faces
    std::vector<vec3f> _norms_; //store all normal line
    std::vector<std::vector<int>> _norms_faces_; //store each face's normal line
    std::vector<std::vector<int>> _tex_faces_;
    std::vector<vec<2>> _tex_coords_;
    TGAImage _diffusemap_;
    TGAImage _normalmap_;

public:
    Model(const char *filename);
    ~Model(); //default

    //interfaces
    int getVertsNumber() const; //maybe useless, get the full number of verts
    int getFacesNumber() const; //get the full number of verts
    const vec3f& getVertsFromIndex(int index) const; //get the verts information
    const std::vector<int>& getFacesFromIndex(int index) const; //get the faces information
    const vec3f &getNormalLineFrom(int face, int verts) const;
    const vec<2> &getTexCoord(int face, int vert) const;
    TGAColor diffuse(vec<2> uv) const;
    vec3f normal(vec<2> uv) const;

protected:
    bool setVarFromObj(const char *filename); //support function in create function
    
};