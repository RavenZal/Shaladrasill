#include "tgaimage.h"
#include "model.h"
#include "definition.h"
#include <cmath>
#include <iostream>
#include <tuple>

using namespace std;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

constexpr int width  = 800;
constexpr int height = 800;
constexpr int deepth = 800;

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer , TGAColor color)
{   
    //set t as varible:
    // for (float t=0.; t<1. ; t+=.02 )
    // {   
    //     int tx = std::round( ax * ( 1 - t ) + bx * t );
    //     int ty = std::round( ay * ( 1 - t ) + by * t );
    //     framebuffer.set(tx, ty, color);
    // }   
    
    //set x as varible
    bool isSteep = std::abs( ax - bx ) < std::abs( ay - by );
    
    if(isSteep) //ensure that the "step" can fill the block
    {
        std::swap(ax, ay);
        std::swap(bx, by);  
    }

    if(ax > bx) //ensure that x in left must be smaller than x in right , y as well
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    
    int ty = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++)
    {
        int tx = x;
        if(isSteep)
        {   
            framebuffer.set(ty, tx, color);
        }else
        {
            framebuffer.set(tx, ty, color);
        }
        ierror += 2 * std::abs(by-ay) ;
        //threshold and judge
        if(ierror > bx - ax)
        {
            ty += by > ay ? 1 : -1 ;
            ierror -= 2 * (bx - ax); //reset error's coordinates system
        }
    }
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) 
{
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void fillTriangleManually(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color)
{
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    //the if's workmode below is not clear:
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) 
    {
        for (int y=bbminy; y<=bbmaxy; y++) 
        {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            if (z <= zbuffer.get(x, y)[0]) continue;
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

std::tuple<int,int,int> project(vec3f v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             (v.z + 1.) *   255./2 };
}

int main(int argc, char** argv) 
{
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);
    Model ModelObject("diablo3_pose.obj");
    //set var
    int ax, bx, cx, ay, by, cy, az, bz, cz = 0; 
    //draw lines
    int FacesNum = ModelObject.getFacesNumber();
    for(int i = 0 ; i < FacesNum ; i++)
    {
        std::vector<int> nowFaceIs = ModelObject.getFacesFromIndex(i);
        vec3f point_a = ModelObject.getVertsFromIndex(nowFaceIs[0]);
        vec3f point_b = ModelObject.getVertsFromIndex(nowFaceIs[1]);
        vec3f point_c = ModelObject.getVertsFromIndex(nowFaceIs[2]);
        ax = ( point_a.x + _ObjModel_Correction_Factor_ ) * width / _ObjModel_Nomalization_ ;
        ay = ( point_a.y + _ObjModel_Correction_Factor_ ) * height / _ObjModel_Nomalization_ ;
        az = ( point_a.z + _ObjModel_Correction_Factor_ ) * deepth / _ObjModel_Nomalization_ ; 
        bx = ( point_b.x + _ObjModel_Correction_Factor_ ) * width / _ObjModel_Nomalization_ ;
        by = ( point_b.y + _ObjModel_Correction_Factor_ ) * height / _ObjModel_Nomalization_ ;
        bz = ( point_b.z + _ObjModel_Correction_Factor_ ) * deepth / _ObjModel_Nomalization_ ;
        cx = ( point_c.x + _ObjModel_Correction_Factor_ ) * width / _ObjModel_Nomalization_ ;
        cy = ( point_c.y + _ObjModel_Correction_Factor_ ) * height / _ObjModel_Nomalization_ ;
        cz = ( point_c.z + _ObjModel_Correction_Factor_ ) * deepth / _ObjModel_Nomalization_ ;
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255; //use random color each triangle
        //line(ax, ay, bx, by, bufferIgnoreZ, white); //outwards
        fillTriangleManually(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}