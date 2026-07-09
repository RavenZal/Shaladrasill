#include "GL.h"
#include <algorithm>
#include <iostream>

mat<4,4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

void lookat(const vec<3> eye, const vec<3> center, const vec<3> up) 
{
    vec<3> _n = eye-center;
    vec<3> _l = cross(up,_n);
    vec<3> _m = cross(_n, _l);
    vec<3> n = _n.normalize();
    vec<3> l = _l.normalize();
    vec<3> m = _m.normalize();
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void init_perspective(const double f)
{
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void init_viewport(const int x, const int y, const int w, const int h) 
{
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector<double>(width*height, -std::numeric_limits<double>::max());
}

void rasterize(const Triangle &clip , const IShader &shader , TGAImage &framebuffer)
{
    vec<4> ndc[3] = {clip[0] / clip[0].w , clip[1] / clip[1].w , clip[2] / clip[2].w};
    vec<2> screen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy()};
    mat<3,3> ABC = { vec<3>(screen[0].x, screen[0].y, 1.),
                vec<3>(screen[1].x, screen[1].y, 1.),
                vec<3>(screen[2].x, screen[2].y, 1.) };

    if (ABC.det() < 1) return;
    //range
    auto [bbminx, bbmaxx] = std::minmax({ screen[0].x, screen[1].x, screen[2].x });
    auto [bbminy, bbmaxy] = std::minmax({ screen[0].y, screen[1].y, screen[2].y });
#pragma omp parallel for
    for (int x= std::max<int>(bbminx, 0 ); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++) 
    {
        for (int y= std::max<int>(bbminy, 0 ); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++) 
        {
            //centre of gravity
            vec<3> bc = invert_transpose(ABC) * vec<3>(static_cast<double>(x), static_cast<double>(y), 1.); //centre of gravity：(Alpha, Beta , Gamma) //screen
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue; //out of this triangle
            double z = bc * vec<3>(ndc[0].z, ndc[1].z, ndc[2].z); //insert
            //if this triangle is behind another one
            if (z <= zbuffer[x + y * framebuffer.width()]) continue;
            int pixelIndex = x + y * framebuffer.width();
            auto [discard, color] =
            shader.fragment(bc, pixelIndex); //should keep this tri->fragment or not 
            if (discard) continue;
            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}
