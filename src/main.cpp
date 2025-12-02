#include "tgaimage.h"
#include "model.h"
#include <cmath>
#include <iostream>

using namespace std;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

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

int main(int argc, char** argv) 
{
    constexpr int width  = 800;
    constexpr int height = 800;
    constexpr int deepth = 800;
    TGAImage bufferIgnoreZ(width, height, TGAImage::RGB);
    TGAImage bufferIgnoreY(width, height, TGAImage::RGB);
    TGAImage bufferIgnoreX(width, height, TGAImage::RGB);
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
        ax = ( point_a.x + 0.5 ) * width;
        ay = ( point_a.y + 0.5 ) * height;
        az = ( point_a.z + 0.5 ) * deepth; 
        bx = ( point_b.x + 0.5 ) * width;
        by = ( point_b.y + 0.5 ) * height;
        bz = ( point_b.z + 0.5 ) * deepth;
        cx = ( point_c.x + 0.5 ) * width;
        cy = ( point_c.y + 0.5 ) * height;
        cz = ( point_c.z + 0.5 ) * deepth;
        line(ax, ay, bx, by, bufferIgnoreZ, red);
        line(ax, ay, cx, cy, bufferIgnoreZ, red);
        line(cx, cy, bx, by, bufferIgnoreZ, red);

        line(ax, az, bx, bz, bufferIgnoreY, red);
        line(ax, az, cx, cz, bufferIgnoreY, red);
        line(cx, cz, bx, bz, bufferIgnoreY, red);

        line(az, ay, bz, by, bufferIgnoreX, red);
        line(az, ay, cz, cy, bufferIgnoreX, red);
        line(cz, cy, bz, by, bufferIgnoreX, red);
    }


    bufferIgnoreZ.write_tga_file("diablo-noZ.tga");
    bufferIgnoreY.write_tga_file("diablo-noY.tga");
    bufferIgnoreX.write_tga_file("diablo-noX.tga");

    return 0;
}