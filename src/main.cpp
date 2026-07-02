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

extern mat<4,4> ModelView, Perspective; // like OpenGL
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader {
    const Model &model;
    //preSet
    TGAColor color = {};
    vec<3> tri[3];  // triangle in eye coordinates
    //light
    vec3f l = vec3f(1, 1, 1).normalize(); 
    double ka = 0.1; // ambient term 
    double kd = 0.5; // diffuse term
    double ks = 0.2; // specular term
    double shininess = 32.0;
    //base color
    vec3f base_color = vec3f (180, 180, 180); //grey
    //tri normal line
    vec3f thisTriNormlineSet[3];
    RandomShader(const Model &m) : model(m) {
    }

    virtual vec<4> vertex(const int face, const int vert) {
        const std::vector<int>& faceSet = model.getFacesFromIndex(face);
        int vertexIndex = faceSet[vert]; 
        vec<3> v = model.getVertsFromIndex(vertexIndex);                       // current vertex in object coordinates
        vec<3> n = model.getNormalLineFrom(face, vert); //get line norm
        vec<4> gl_Position = ModelView * vec<4>{v.x, v.y, v.z, 1.};
        vec<4> gl_Normal = ModelView * vec<4>{n.x, n.y, n.z, 0.}; 
        tri[vert] = {gl_Position[0], gl_Position[1], gl_Position[2]};                            // in eye coordinates
        thisTriNormlineSet[vert] = vec3f (gl_Normal[0], gl_Normal[1], gl_Normal[2]).normalize();
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec<3> bar) const {
        vec3f n = cross(tri[1] - tri[0] , tri[2] - tri[0]).normalize();
        vec3f center_poiont_vec = (tri[0] + tri[1] + tri[2]) / 3.0 ;
        vec3f v = (vec3f(0,0,0) - center_poiont_vec).normalize();
        //diffuse
        double unit_dif = std::max(0., l * n);
        double diffuse = kd * unit_dif;
        //ambient
        double ambient = ka;
        //specular
        double specular = 0.0;       
        if(unit_dif > 0.0)
        {
            vec3f r = ((2 * (n * l) * n) - l).normalize() ;
            specular = ks * std::pow(std::max(0.0, r * v), shininess);
        }
        //total intensity
        double intensity = std::clamp(ambient + diffuse + specular, 0.0, 1.0);
        unsigned char r = static_cast<unsigned char>(std::clamp(base_color.x * intensity, 0.0, 255.0));
        unsigned char g = static_cast<unsigned char>(std::clamp(base_color.y * intensity, 0.0, 255.0));
        unsigned char b = static_cast<unsigned char>(std::clamp(base_color.z * intensity, 0.0, 255.0));
        return {false, TGAColor{b , g , r , 255}};                                    // do not discard the pixel
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
