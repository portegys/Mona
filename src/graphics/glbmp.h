/******************************************************************************
* glbmp - a library for loading Windows & OS/2 bitmaps for use in OpenGL      *
* Copyright (C) 2005 Charles Lindsay                                          *
*                                                                             *
*  This software is provided 'as-is', without any express or implied          *
*  warranty.  In no event will the authors be held liable for any damages     *
*  arising from the use of this software.                                     *
*                                                                             *
*  Permission is granted to anyone to use this software for any purpose,      *
*  including commercial applications, and to alter it and redistribute it     *
*  freely, subject to the following restrictions:                             *
*                                                                             *
*  1. The origin of this software must not be misrepresented; you must not    *
*     claim that you wrote the original software. If you use this software    *
*     in a product, an acknowledgment in the product documentation would be   *
*     appreciated but is not required.                                        *
*  2. Altered source versions must be plainly marked as such, and must not be *
*     misrepresented as being the original software.                          *
*  3. This notice may not be removed or altered from any source distribution. *
*                                                                             *
* The "official" glbmp webpage is: http://chaoslizard.sourceforge.net/glbmp/  *
* Contact the author at: chaoslizard@gamebox.net                              *
******************************************************************************/

/******************************************************************************
* glbmp.h v1.1 (2005 Mar. 15)                                                 *
*                                                                             *
* changes                                                                     *
* -------                                                                     *
* 2005-03-14/CL - Initial version.                                            *
* 2005-03-15/CL - Added byte-swapping code to glbmp.c for big-endian          *
*                 architectures (no changes here, just keeping synchronized). *
******************************************************************************/

#ifndef __glbmp_h__
#define __glbmp_h__

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* glbmp_t
 *
 * The struct returned by glbmp_LoadBitmap.  It has enough information to
 * enable you to use it in OpenGL texture creation.
 */
typedef struct glbmp_t
{
   int           width;                           /* width of bitmap                              */
   int           height;                          /* height of bitmap                             */

   unsigned char *rgb_data;

   /* pointer to buffer, width*height*3 in size,
    *                              holding RGB bitmap data                      */
} glbmp_t;

/* outputs rgb_data as top down (default is bottom first)  */
#define GLBMP_TOP_DOWN      1

/* performs no dword alignment (default aligns lines on dword boundaries) */
#define GLBMP_BYTE_ALIGN    2

/* allows loading of any size bitmap (default is bitmaps must be 2^n x 2^m) */
#define GLBMP_ANY_SIZE      4

/* glbmp_LoadBitmap
 *
 * Loads the specified bitmap file from disk and converts the data to a format
 * usable by OpenGL.  The bitmap file must be 1, 4, 8, or 24 bits, and not
 * compressed (this means NO RLE (yet)).
 *
 * Inputs:
 * bmp_file - The filename of the bitmap file to load.
 * flags - One or more GLBMP_* flags (defined above), combined with bitwise or.
 *         Specify 0 for standard, OpenGL compliant behavior.
 * p_bmp_out - Pointer to a glbmp_t struct to fill with information.
 *
 * Returns:
 * 0 if there's an error (file doesn't exist or is invalid, i/o error, etc.),
 * in which case p_bmp_out isn't guaranteed to hold anything in particular, or
 * 1 if the file loaded ok.
 *
 * Notes:
 * Standard behavior is for glbmp_LoadBitmap to return rgb_data starting with
 * the last scan line and ending with the first (this is apparently how OpenGL
 * expects data).  If you're using the data for something else and want normal
 * top-down data, specify GLBMP_TOP_DOWN in flags.  Also, glbmp_LoadBitmap will
 * normally fail if the bitmap has width or height that isn't a power of 2.  To
 * ignore this restriction (though your bitmap may not render in OpenGL),
 * specify GLBMP_ANY_SIZE in flags.  Finally, glbmp_LoadBitmap will DWORD align
 * the scan lines by default (only very small files need padding, unless you
 * specify GLBMP_ANY_SIZE), but again if you need byte-packed data, specify
 * GLBMP_BYTE_ALIGN in flags (may break OpenGL though).
 */
int glbmp_LoadBitmap(const char *bmp_file, int flags, glbmp_t *p_bmp_out);

/* glbmp_FreeBitmap
 *
 * Frees memory allocated during glbmp_LoadBitmap.  Call glbmp_FreeBitmap when
 * you are done using the glbmp_t struct (i.e., after you have passed the data
 * on to OpenGL).
 *
 * Inputs:
 * p_bmp - The pointer returned by glbmp_LoadBitmap.
 *
 * Returns:
 * void
 */
void glbmp_FreeBitmap(glbmp_t *p_bmp);

#ifdef __cplusplus
}
#endif
#endif
