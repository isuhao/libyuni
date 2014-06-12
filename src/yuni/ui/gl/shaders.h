#ifndef __YUNI_UI_GL_SHADERS_H__
# define __YUNI_UI_GL_SHADERS_H__

// This file contains necessary GLSL shaders for internal use

namespace Yuni
{
namespace Gfx3D
{



	/////////////// VERTEX SHADERS


	// Minimal vertex shader : only transform the vertex coordinates
	extern const char* const vsTransform;

	// Very simple vertex shader : transform coordinates and propagate texture coordinates
	extern const char* const vsTexCoord;

	// For 2D post shaders, texture coordinates are calculated by transforming vertex position
	// from [-1,1] to [0,1]
	extern const char* const vs2D;

	// Pass the color as attribute
	extern const char* const vsColorAttr;

	// Sample a texture using a rectangle, do not resize the image, fill empty parts with a color
	extern const char* const vsImageRect;

	// Phong shading
	extern const char* const vsPhong;

	extern const char* const vsCubeMap;



	/////////////// FRAGMENT SHADERS


	// Use a single color given as uniform
	extern const char* const fsColorUniform;

	// Use a single color given as attribute
	extern const char* const fsColorAttr;

	// Use directly the texture value, no lighting
	extern const char* const fsSimpleTexture;

	// Sample a texture using a rectangle, do not resize the image, fill empty parts with a color
	extern const char* const fsImageRect;

	// Freetype with normal render mode generates alpha-only bitmaps, stored as GL_R textures
	// This shader displays them with the proper color.
	extern const char* const fsText;

	extern const char* const fsTextOnSolidColor;

	// Color picking
	extern const char* const fsPicking;

	// Skybox : cube map sampling
	extern const char* const fsSkybox;

	// Phong shading
	extern const char* const fsPhong;


	//// POST FRAGMENT SHADERS

	extern const char* const fsYuv2Rgb;



	/////////////// GEOMETRY SHADERS


	// Generate empty borders for image rectangles
	extern const char* const gsImageRect;



} // namespace Gfx3D
} // namespace Yuni

#endif // __YUNI_UI_GL_SHADERS_H__
