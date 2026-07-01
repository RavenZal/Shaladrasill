#include "tgaimage.h"
#include "model.h"
#include "definition.h"
#include "GL.h"
#include <cmath>
#include <iostream>
#include <tuple>
#include <limits>
#include <algorithm>

using namespace std;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

constexpr int width  = 800;
constexpr int height = 800;
constexpr float deepth = 255.0;

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader {
    const Model &model;
    TGAColor color = {};
    vec<3> tri[3];  // triangle in eye coordinates

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec<4> vertex(const int face, const int vert) {
        const std::vector<int>& faceSet = model.getFacesFromIndex(face);
        int vertexIndex = faceSet[vert]; 
        vec<3> v = model.getVertsFromIndex(vertexIndex);                       // current vertex in object coordinates
        vec<4> gl_Position = ModelView * vec<4>{v.x, v.y, v.z, 1.};
        tri[vert] = {gl_Position[0], gl_Position[1], gl_Position[2]};                            // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec<3> bar) const {
        return {false, color};                                    // do not discard the pixel
    }
};

int main(int argc, char** argv) 
{
    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model ModelObject("diablo3_pose.obj");
    //draw lines
    int FacesNum = ModelObject.getFacesNumber();
    //set camera
    const vec<3>    eye{-1,0,2}; // camera position
    const vec<3> center{0,0,0};  // camera direction
    const vec<3>     up{0,1,0};  // camera up vector

    lookat(eye, center, up);                              // build the ModelView   matrix
    init_perspective((eye-center).norm());                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);

    //make shader
    RandomShader shader(ModelObject);

    for(int i = 0 ; i < FacesNum ; i++)
    {
        Triangle clip;
        clip[0] = shader.vertex(i, 0);
        clip[1] = shader.vertex(i, 1);
        clip[2] = shader.vertex(i, 2);
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255; //use random color each triangle
        shader.color = rnd;
        rasterize(clip, shader, framebuffer);
    }
    //mapping Z Buffer
    double Zmax = - std::numeric_limits<double>::max();
    double Zmin = std::numeric_limits<double>::max();
    for (int i = 0 ; i < width*height ; i++)
    {
        if(zbuffer[i] > - std::numeric_limits<double>::max())
        {
            Zmin = std::min(Zmin , zbuffer[i]);
            Zmax = std::max(Zmax , zbuffer[i]);
        }
    }
    //Grey Image
    TGAImage depthImage(width, height, TGAImage::GRAYSCALE);
    for (int x = width - 1 ; x >= 0 ; x -- )
    {
         for (int y = height - 1 ; y >= 0 ; y-- )
         {
            double depth = zbuffer[x + y * width];
            if(depth > - std::numeric_limits<double>::max())
            {
                unsigned char gray = 255 * (depth - Zmin) / (Zmax - Zmin);
                depthImage.set(x, y, TGAColor{gray, gray, gray, 255});
            }
         }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    std::cout << "[Main] framebuffer Work Finished" << std::endl;
    depthImage.write_tga_file("zbuffer.tga");
    std::cout << "[Main] zbuffer Work Finished" << std::endl;
    std::cout << "Total faces: " << FacesNum << std::endl;
    return 0;
}
