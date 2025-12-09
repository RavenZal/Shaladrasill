#include "tgaimage.h"
#include "model.h"
#include "definition.h"
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

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) 
{
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void fillTriangleManually(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer)
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
            unsigned char R = alpha * 255;
            unsigned char G = beta * 255;
            unsigned char B = gamma * 255;
            framebuffer.set(x, y, {B,G,R,255});
        }
    }
}

int main(int argc, char** argv) 
{
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGBA);

    int ax = 17, ay =  4, az =  13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    fillTriangleManually(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);

    framebuffer.write_tga_file("shadow.tga");
    return 0;
}