// Create texture function.

#include "texture.h"
#include <stdio.h>

// This function creates a texture from a BMP file 'strFileName'. It further stores the OpenGL handle to the generated texture
// in the 'textureID' location of 'textureArray'.
bool CreateTexture(char *strFileName, GLuint *textureArray, GLint textureID)
{
   glbmp_t bitmap;                                //object to fill with data from glbmp

   if (!strFileName)                              // Return from the function if no file name was passed in
   {
      return(false);
   }

   //try to load the specified file--if it fails, dip out
   if (!glbmp_LoadBitmap(strFileName, GLBMP_ANY_SIZE, &bitmap))
   {
      return(false);
   }

   // Now that we have the texture data, we need to register our texture with OpenGL
   // To do this we need to call glGenTextures().  The 1 for the first parameter is
   // how many texture we want to register this time (we could do a bunch in a row).
   // The second parameter is the array index that will hold the reference to this texture.

   // Generate a texture with the associative texture ID stored in the array
   glGenTextures(1, &textureArray[textureID]);

   // Now that we have a reference for the texture, we need to bind the texture
   // to tell OpenGL this is the reference that we are assigning the bitmap data too.
   // The first parameter tells OpenGL we want are using a 2D texture, while the
   // second parameter passes in the reference we are going to assign the texture too.
   // We will use this function later to tell OpenGL we want to use this texture to texture map.

   // Bind the texture to the texture arrays index and init the texture
   glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

   // Now comes the important part, we actually pass in all the data from the bitmap to
   // create the texture. Here is what the parameters mean in gluBuild2DMipmaps():
   // (We want a 2D texture, 3 channels (RGB), bitmap width, bitmap height, It's an RGB format,
   //  the data is stored as unsigned bytes, and the actuall pixel data);

   // What is a Mip map?  Mip maps are a bunch of scaled pictures from the original.  This makes
   // it look better when we are near and farther away from the texture map.  It chooses the
   // best looking scaled size depending on where the camera is according to the texture map.
   // Otherwise, if we didn't use mip maps, it would scale the original UP and down which would
   // look not so good when we got far away or up close, it would look pixelated.

   // Build Mipmaps (builds different versions of the picture for distances - looks better)
   gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bitmap.width, bitmap.height, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data);

   // Lastly, we need to tell OpenGL the quality of our texture map.  GL_LINEAR_MIPMAP_LINEAR
   // is the smoothest.  GL_LINEAR_MIPMAP_NEAREST is faster than GL_LINEAR_MIPMAP_LINEAR,
   // but looks blochy and pixilated.  Good for slower computers though.  Read more about
   // the MIN and MAG filters in the texture.cpp.

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

   // Now we need to free the bitmap data that we loaded since openGL stored it as a texture
   glbmp_FreeBitmap(&bitmap);

   return(true);
}


// Load targa image texture.
bool LoadTGA(TextureImage *texture, char *filename)
{
   GLubyte TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   GLubyte TGAcompare[12];
   GLubyte header[6];
   GLuint  bytesPerPixel;
   GLuint  imageSize;
   GLuint  type = GL_RGBA;

   FILE *file = fopen(filename, "rb");

   if ((file == NULL) ||
       (fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare)) ||
       (memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0) ||
       (fread(header, 1, sizeof(header), file) != sizeof(header)))
   {
      if (file == NULL)
      {
         return(false);
      }
      else
      {
         fclose(file);
         return(false);
      }
   }

   texture->width  = header[1] * 256 + header[0];
   texture->height = header[3] * 256 + header[2];

   if ((texture->width <= 0) ||
       (texture->height <= 0) ||
       ((header[4] != 24) && (header[4] != 32)))
   {
      fclose(file);
      return(false);
   }

   texture->bpp  = header[4];
   bytesPerPixel = texture->bpp / 8;
   imageSize     = texture->width * texture->height * bytesPerPixel;

   texture->imageData = new GLubyte[imageSize];

   if ((texture->imageData == NULL) ||
       (fread(texture->imageData, 1, imageSize, file) != imageSize))
   {
      if (texture->imageData != NULL)
      {
         delete [] texture->imageData;
      }

      fclose(file);
      return(false);
   }

   for (unsigned int k = 0; k < imageSize; k += bytesPerPixel)
   {
      texture->imageData[k] ^= texture->imageData[k + 2] ^=
                                  texture->imageData[k] ^= texture->imageData[k + 2];
   }

   fclose(file);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glGenTextures(1, &texture[0].texID);

   glBindTexture(GL_TEXTURE_2D, texture[0].texID);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   if (texture[0].bpp == 24)
   {
      type = GL_RGB;
   }

   glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

   return(true);
}
