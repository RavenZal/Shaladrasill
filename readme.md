# 🎨 TinyRenderer

> A from-scratch CPU software rasterizer — no OpenGL, no GPU, just math and pixels.

**TinyShader** renders 3D models entirely on the CPU. It reads OBJ geometry and TGA textures, transforms vertices through a full graphics pipeline, rasterizes triangles by hand, shades every pixel with tangent-space normal mapping and Phong lighting, casts shadows, and applies screen-space ambient occlusion — all in C++.

Built following the [tinyrenderer](https://haqr.eu/tinyrenderer/) tutorial series.

---

## 🖼️ What it produces

| Output | Content |
|---|---|
| `framebuffer.tga` | Final shaded image with shadows + SSAO |
| `normalbuffer.tga` | View-space normal visualization |
| `zbuffer.tga` | Light-view depth map (shadow map) |

---

## 🧩 Rendering Pipeline

```
OBJ + Textures
  │
  ├─► Pass 1: Shadow Map ─────► shadowbuffer
  │     (DepthShader from light)
  │
  ├─► Pass 2: SSAO ───────────► aoBuffer
  │     Camera depth + normals
  │     32 screen-space samples
  │     Hemisphere kernel with range weighting
  │
  └─► Pass 3: Final Shading ──► framebuffer.tga
        Phong lighting
        Tangent-space normal mapping
        Shadow visibility query
        SSAO ambient factor
```

---

## ✨ Features

| Category | What's implemented |
|---|---|
| **Coordinate pipeline** | `Object → Eye → Clip → NDC → Screen` via `ModelView × Perspective × Viewport` |
| **Rasterizer** | Barycentric triangle coverage, Z-buffer depth test, `IShader` programmable interface |
| **Shading** | Phong specular (not Blinn-Phong) with ambient + diffuse + specular terms |
| **Normal mapping** | Tangent-space TBN matrix; samples from tangent normal map, transforms to view space |
| **Shadow mapping** | Light-view depth pass → per-pixel visibility query with depth bias |
| **SSAO** | Screen-space hemisphere sampling, view-position reconstruction, range-based weighting, bias guard |
| **Perspective correction** | `1/w` interpolation for shadow coordinates and camera normals |
| **Textures** | TGA I/O (RGB + grayscale), BGRA-aware sampling with UV flipping |

---

## 📁 Project Map

```
├── assets/                          # Diablo 3 pose model + 3 textures
│   ├── diablo3_pose.obj
│   ├── diablo3_pose_diffuse.tga
│   ├── diablo3_pose_nm.tga          # Object-space normal (unused in final)
│   └── diablo3_pose_nm_tangent.tga  # Tangent-space normal (active)
├── include/
│   ├── geometry.h                   # vec/mat math (custom, header-only)
│   ├── GL.h                         # Camera, rasterizer, IShader interface
│   ├── model.h                      # OBJ loader, texture sampling
│   └── tgaimage.h                   # TGA read/write
├── src/
│   ├── main.cpp                     # Shaders + full render orchestration
│   ├── GL.cpp                       # lookat, perspective, viewport, rasterize
│   ├── model.cpp                    # OBJ parsing, tangent-space normal calc
│   └── tgaimage.cpp                 # TGA file I/O
└── CMakeLists.txt                   # C++17, single executable target
```

---

## 🔧 Build & Run

```bash
cmake -S . -B build
cmake --build build --parallel
./build/TinyShader
```

> Run from the project root — asset paths are relative.

Requires a **C++17** compiler. No external dependencies beyond the standard library.

---

## 📐 Coordinate Spaces at a Glance

| Space | How we get there |
|---|---|
| **Object** | Raw OBJ vertex positions |
| **Eye (View)** | `ModelView × object` |
| **Clip** | `Perspective × eye` — 4D homogeneous, **keep `w`** |
| **NDC** | `clip / clip.w` — perspective divide |
| **Screen** | `Viewport × ndc` — pixel grid |

---

## 🔬 Key Algorithms

**SSAO** — Reconstructs view-space position from screen `(x, y, depth)`, generates a 32-sample hemisphere kernel oriented to the surface normal via a TBN-style frame, projects each sample back to screen space, and compares depth. A range-based falloff weight (`1 − dist/radius`) prevents distant surfaces from counting as occluders.

**Shadow Test** — Transforms the shaded point into light clip space, applies perspective divide + viewport, then compares against the light's Z-buffer. Points behind the stored depth are in shadow (visibility = 0.05).

**TBN Construction** — `T` and `B` are derived from triangle edges and UV deltas in view space; `N` is the interpolated vertex normal. The tangent normal map sample `(nx, ny, nz)` becomes `T·nx + B·ny + N·nz`.

---

## 🧪 SSAO Kernel (visual intuition)

```
         N (surface normal)
         │  ● sample (in hemisphere)
         │ ╱
    ─────●─────  surface
```

Kernel samples are randomly placed in a unit hemisphere, scaled by an accelerating curve (`lerp(0.1, 1.0, i²/n²)`), then aligned to the surface normal. This biases samples closer to the center for more relevant occlusion.

---

## 📊 Evolution

| Lesson | Concept | Status |
|---|---|---|
| Better Camera | Unified matrix transform chain | ✅ |
| Shading | `IShader` + Phong lighting | ✅ |
| Textures | UV sampling, diffuse map | ✅ |
| Tangent Space | TBN matrix, normal mapping | ✅ |
| Shadow Mapping | Light-view depth + visibility | ✅ |
| SSAO | Screen-space ambient occlusion | ✅ |

---

## 🙏 Credits
Deeply appreciate: Prof. Dmitry V. Sokolov [ssloy](https://github.com/ssloy)

Based on Course [Playing with code](https://haqr.eu/tinyrenderer/) — a brilliant course in building a software renderer from scratch.

Model and textures from Blizzard's Diablo 3, Diablo, used for educational purposes.
