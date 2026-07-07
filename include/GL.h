#include "tgaimage.h"
#include "geometry.h"

void lookat(const vec3f eye, const vec3f center, const vec3f up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    virtual vec<4> vertex(const int face, const int vert) = 0;
    virtual std::pair<bool,TGAColor> fragment(const vec3f bar, const int pixelIndex) const = 0;
    virtual ~IShader() = default;
};

typedef vec<4> Triangle[3]; // a triangle primitive is made of three ordered points
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);