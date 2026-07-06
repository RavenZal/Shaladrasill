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

extern mat<4,4> ModelView, Perspective, Viewport; // like OpenGL
extern std::vector<double> zbuffer;     // the depth buffer

vec3f light_dir;
vec3f light_eye;

mat<4,4> ShadowModelView ;
mat<4,4> ShadowPerspective ;
mat<4,4> ShadowViewport ;
    
struct DepthShader : IShader{
    const Model &model;
    //preSet
    TGAColor color = {};
    DepthShader(const Model &m) : model(m) {
    }

    virtual vec<4> vertex(const int face, const int vert) {
    const std::vector<int>& faceSet = model.getFacesFromIndex(face);
    int vertexIndex = faceSet[vert]; 
    vec<3> v = model.getVertsFromIndex(vertexIndex);                       // current vertex in object coordinates
    vec<3> n = model.getNormalLineFrom(face, vert); //get line norm
    vec<4> gl_Position = ModelView * vec<4>{v.x, v.y, v.z, 1.};
    vec<4> gl_Normal = ModelView * vec<4>{n.x, n.y, n.z, 0.}; 
    return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec<3> bar) const {
        unsigned char b = rand() %255;
        unsigned char g = rand() %255;
        unsigned char r = rand() %255;
        return {false, TGAColor{b , g , r , 255}};                                    // do not discard the pixel
    }


};
struct RandomShader : IShader {
    const Model &model;
    vec3f l;
    const std::vector<double> &_shadowbuffer;
    //preSet
    TGAColor color = {};
    vec<3> tri[3];  // triangle in eye coordinates
    //light
    double ka = 0.3; // ambient term 
    double kd = 0.85; // diffuse term
    double ks = 0.20; // specular term
    double shininess = 32.0;
    //base color
    vec3f base_color = vec3f (180, 180, 180); //grey
    //tri normal line
    vec3f thisTriNormlineSet[3];
    //tri texture
    vec<2> thisTriTexSet[3];
    //tri shadow
    vec<4> shadowClip[3]; //in clip shadow coor, without /w
    double cameraInvW[3]; //with /w
    //in shadowTri : x = x in shadow map, y = y in shadow map
    //z = deepth in light view
    RandomShader(const Model &m, const std::vector<double>& shadowbuffer) : model(m), _shadowbuffer(shadowbuffer) 
    {
        //l
        vec<4> _l;
        l = light_dir;
        _l = ModelView * vec<4>{l.x , l.y , l.z , 0.};
        _l = Perspective * _l; 
        l = {_l.x , _l.y , _l.z};
    }

    virtual vec<4> vertex(const int face, const int vert) {
        const std::vector<int>& faceSet = model.getFacesFromIndex(face);
        int vertexIndex = faceSet[vert]; 
        vec<2> uv = model.getTexCoord(face , vert);
        vec<3> v = model.getVertsFromIndex(vertexIndex);                       // current vertex in object coordinates
        vec<3> n = model.getNormalLineFrom(face, vert); //get line norm
        vec<4> gl_Position = ModelView * vec<4>{v.x, v.y, v.z, 1.};
        vec<4> gl_Normal = ModelView * vec<4>{n.x, n.y, n.z, 0.}; 
        tri[vert] = {gl_Position[0], gl_Position[1], gl_Position[2]};                            // in eye coordinates
        thisTriNormlineSet[vert] = vec3f (gl_Normal[0], gl_Normal[1], gl_Normal[2]).normalize();
        thisTriTexSet[vert] = uv;
        //keep var in shadow tri
        vec<4> camera_clip = Perspective * gl_Position;
        shadowClip[vert] =
            ShadowPerspective *
            ShadowModelView *
            vec<4>{v.x, v.y, v.z, 1.0};
       cameraInvW[vert] = 1.0 / camera_clip.w;
        //return
        return camera_clip;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec<3> bar ) const {
        //shadow_pt
        vec3f correctedBar{
            bar.x * cameraInvW[0],
            bar.y * cameraInvW[1],
            bar.z * cameraInvW[2]
        };
        double correctedSum =
            correctedBar.x +
            correctedBar.y +
            correctedBar.z;

        correctedBar = correctedBar / correctedSum;
        vec<4> light_clip =
        shadowClip[0] * correctedBar.x
        + shadowClip[1] * correctedBar.y
        + shadowClip[2] * correctedBar.z;
        vec<4> light_ndc = light_clip / light_clip.w;
        vec<2> light_xy = (ShadowViewport * light_ndc).xy();
        vec3f shadowPt{
            light_xy.x,
            light_xy.y,
            light_ndc.z
        };
        //check
        double visibility = 1.0;

        bool insideShadowMap =
            shadowPt.x >= 0.0
            && shadowPt.x < static_cast<double>(width)
            && shadowPt.y >= 0.0
            && shadowPt.y < static_cast<double>(height);

        if (insideShadowMap)
        {
            int shadowX = static_cast<int>(shadowPt.x);
            int shadowY = static_cast<int>(shadowPt.y);

            double shadowDepth =
                _shadowbuffer[shadowX + shadowY * width];

            if (shadowDepth > -std::numeric_limits<double>::max())
            {
                double bias = 1e-2;

                bool in_shadow =
                    shadowPt.z < shadowDepth - bias;

                visibility = in_shadow ? 0.05 : 1.0;
            }
        }

        vec3f interpolated_n = (thisTriNormlineSet[0] * bar.x
                  +thisTriNormlineSet[1] * bar.y
                  +thisTriNormlineSet[2] * bar.z
        ).normalize();

        //TGN
        vec3f N = interpolated_n ;
        vec3f E1 = tri[1] - tri[0];
        vec3f E2 = tri[2] - tri[0];

        vec<2> dUV1 = thisTriTexSet[1] - thisTriTexSet[0];
        vec<2> dUV2 = thisTriTexSet[2] - thisTriTexSet[0];
        double det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
        vec3f T = (E1 * dUV2.y - E2 * dUV1.y) / det;
        vec3f B = (E2 * dUV1.x - E1 * dUV2.x) / det;
        vec<2> uv = (thisTriTexSet[0] * bar.x
                    +thisTriTexSet[1] * bar.y
                    +thisTriTexSet[2] * bar.z
        );

        vec3f n_tangent = model.normal_tangent(uv);
        vec3f fragPos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;
        vec3f v = (vec3f(0,0,0) - fragPos).normalize();
        //vec3f n = model.normal(uv);
        vec3f n = (T * n_tangent.x +
                   B * n_tangent.y +
                   N * n_tangent.z).normalize();
        //diffuse
        double unit_dif = std::max(0., l * n);
        double diffuse = kd * unit_dif;
        //ambient
        double ambient = ka;
        //specular
        double specular = ks;       
        if(unit_dif > 0.0)
        {
            vec3f r = ((2 * (n * l) * n) - l).normalize() ;
            specular = ks * std::pow(std::max(0.0, r * v), shininess);
        }
        //color Texture
        TGAColor tex = model.diffuse(uv);
        vec3f albedo(tex[2], tex[1], tex[0]);
        //total intensity
        double intensity = std::clamp(ambient + visibility * (diffuse + specular), 0.0, 1.0);
        unsigned char r = static_cast<unsigned char>(std::clamp(albedo.x * intensity, 0.0, 255.0));
        unsigned char g = static_cast<unsigned char>(std::clamp(albedo.y * intensity, 0.0, 255.0));
        unsigned char b = static_cast<unsigned char>(std::clamp(albedo.z * intensity, 0.0, 255.0));
        return {false, TGAColor{b , g , r , 255}};                                    // do not discard the pixel
    }
};

int main(int argc, char** argv) 
{
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage Tempbuffer(width, height, TGAImage::RGB);
    Model ModelObject("diablo3_pose.obj");
    //draw lines
    int FacesNum = ModelObject.getFacesNumber();
    //set camera
    const vec<3>    eye{-1,0,2}; // camera position
    const vec<3> center{0,0,0};  // camera direction
    const vec<3>     up{0,1,0};  // camera up vector
    light_dir = vec3f(1 , 1 , 1).normalize();
    light_eye = center + light_dir * 5;


    //first
    lookat(light_eye, center, up);                              // build the ModelView   matrix
    init_perspective((light_eye - center).norm());                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);

    DepthShader depthShader(ModelObject);
    for (int i = 0 ; i < FacesNum ; i++) {
    Triangle clip;
    clip[0] = depthShader.vertex(i, 0);
    clip[1] = depthShader.vertex(i, 1);
    clip[2] = depthShader.vertex(i, 2);
    rasterize(clip, depthShader, Tempbuffer);
    }
    std::vector<double> shadowbuffer = zbuffer;
    ShadowModelView = ModelView;
    ShadowPerspective = Perspective;
    ShadowViewport = Viewport;
    
    //sencond
    lookat(eye, center, up);                              // build the ModelView   matrix
    init_perspective((eye-center).norm());                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);

    RandomShader shader(ModelObject, shadowbuffer);

    for(int i = 0 ; i < FacesNum ; i++)
    {
        Triangle clip;
        clip[0] = shader.vertex(i, 0);
        clip[1] = shader.vertex(i, 1);
        clip[2] = shader.vertex(i, 2);
        rasterize(clip, shader, framebuffer);
    }
    //mapping Z Buffer
    double Zmax = - std::numeric_limits<double>::max();
    double Zmin = std::numeric_limits<double>::max();
    for (int i = 0 ; i < width*height ; i++)
    {
        if(shadowbuffer[i] > - std::numeric_limits<double>::max())
        {
            Zmin = std::min(Zmin , shadowbuffer[i]);
            Zmax = std::max(Zmax , shadowbuffer[i]);
        }
    }
    //Grey Image
    TGAImage depthImage(width, height, TGAImage::GRAYSCALE);
    for (int x = width - 1 ; x >= 0 ; x -- )
    {
         for (int y = height - 1 ; y >= 0 ; y-- )
         {
            double depth = shadowbuffer[x + y * width];
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
