/***************************************************************************************************************/

/* HEADER FILE FOR THE TUTORIAL ON TEXTURE MAPPING.
 * /*	Dated : May 27th 2004						Author: Ramgopal R,
 * /*												Computer Graphics and Visualization Lab. Concordia University
 * /*												e-mail: ramgopal@acm.org
 * /*												URL: http://www.snipurl.com/ramgopal/graphics
 * /*
 * /*Notes and Acknowledgements:					Thanks to DigiBen from GameTutorials.com for a significant
 * /*												portion of the write up.
 * /**************************************************************************************************************/

#ifndef __TEXTURE_H
#define __TEXTURE_H

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "glbmp.h"

// This function creates a texture from a BMP file 'strFileName'. It further stores the OpenGL handle to the generated texture
// in the 'textureID' location of 'textureArray'.
bool CreateTexture(char *strFileName, GLuint *textureArray, GLint textureID);

typedef struct
{
   GLubyte *imageData;
   GLuint  bpp;
   GLuint  width;
   GLuint  height;
   GLuint  texID;
} TextureImage;

// Load targa image texture.
bool LoadTGA(TextureImage *texture, char *filename);
#endif
