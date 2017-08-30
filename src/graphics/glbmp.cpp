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
* glbmp.c v1.1 (2005 Mar. 15)                                                 *
*                                                                             *
* changes                                                                     *
* -------                                                                     *
* 2005-03-14/CL - Initial version.                                            *
* 2005-03-15/CL - Added byte-swapping code (for big-endian architectures).    *
******************************************************************************/

/******************************************************************************
* I'm pretty sure this source is 100 per cent ANSI C compliant, and therefore *
* portable to just about everywhere in the universe.  Let me know if I'm      *
* wrong.                                                                      *
******************************************************************************/

#include "glbmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MSVC++ and some others don't have stdint.h.  These typedefs USUALLY work. */
#if defined(_MSC_VER)
typedef unsigned char    uint8_t;                 /* must be 8 bits, unsigned */
typedef unsigned short   uint16_t;                /* must be 16 bits, unsigned */
typedef unsigned long    uint32_t;                /* must be 32 bits, unsigned */
typedef          long    int32_t;                 /* must be 32 bits, signed */
#else
/* inttypes.h should include stdint.h or int_types.h */
#include <inttypes.h>
#if !defined(_STDINT_H) && !defined(_SYS_INT_TYPES_H)
typedef unsigned char    uint8_t;                 /* must be 8 bits, unsigned */
typedef unsigned short   uint16_t;                /* must be 16 bits, unsigned */
typedef unsigned long    uint32_t;                /* must be 32 bits, unsigned */
typedef          long    int32_t;                 /* must be 32 bits, signed */
#endif
#endif

/* Hack to determine endianness of architecture, to byte swap if necessary.  */
/* If my list is incomplete, just define __LITTLE_ENDIAN__ or don't.         */
#if (defined(__i386__) || defined(__i386) || defined(__x86_64__)     \
   || defined(__alpha__) || defined(__alpha) || defined(__ia64__)    \
   || defined(_WIN32) || defined(WIN32) || defined(_WIN64)           \
   || defined(__arm__) || (defined(__mips__) && defined(__MIPSEL__)) \
   || defined(__SYMBIAN32__) || defined(__LITTLE_ENDIAN__))
#undef _GLBMP_BYTESWAP     //undef to do no swapping
#else
#define _GLBMP_BYTESWAP    //tell glbmp to swap bytes
#endif

typedef struct _bmp_header                        /* first few bytes of file, after magic word "BM" */
{
   uint32_t file_size;                            /* size of whole file */
   uint32_t unused;                               /* should be 0 */
   uint32_t data_offset;                          /* offset from beginning of file to bitmap data */
} _bmp_header;

typedef struct _bmp_info                          /* immediately follows header; describes image */
{
   uint32_t info_size;                            /* size of info struct (is >sizeof(_bmp_info)) */
   int32_t  width;                                /* width of image */
   int32_t  height;                               /* height (< 0 means right-side up) */
   uint16_t planes;                               /* planes (should be 1) */
   uint16_t bits;                                 /* number of bits (1, 4, 8, 16, 24, or 32) */
   uint32_t compression;                          /* 0 = none, 1 = 8-bit RLE, 2 = 4-bit RLE */
   /* there are other fields in the actual file info, but we don't need 'em */
} _bmp_info;

typedef struct _bmp_palette_entry                 /* a single color in the palette */
{
   uint8_t blue;                                  /* blue comes first, for some reason */
   uint8_t green;                                 /* green component */
   uint8_t red;                                   /* red comes last */
   uint8_t unused;                                /* one unused byte.  Great. */
} _bmp_palette_entry;

typedef struct _bmp_read_context                  /* data passed around between read ops */
{
   FILE               *fp;                        /* file pointer */
   _bmp_header        header;                     /* file header */
   _bmp_info          info;                       /* file info */
   int                lines;                      /* how many scan lines (abs(height)) */
   int                file_line_len;              /* how many bytes each scan line is */
   int                rgb_line_len;               /* how many bytes each output line is */
   _bmp_palette_entry *palette;                   /* palette */
   uint8_t            *file_data;                 /* a line of data in the file */
   uint8_t            *rgb_data;                  /* rgb data output buffer */
} _bmp_read_context;

/* prototypes -- see their implementations for details */
static int _bmp_ReadHeader(_bmp_read_context *p_ctx);
static int _bmp_ReadInfo(_bmp_read_context *p_ctx, int flags);
static int _bmp_InitDecode(_bmp_read_context *p_ctx);
static int _bmp_Decode(_bmp_read_context *p_ctx, int flags);
static void _bmp_FreeContext(_bmp_read_context *p_ctx, int leave_rgb_data);

/* see header for details */
int glbmp_LoadBitmap(const char *bmp_file, int flags, glbmp_t *p_bmp_out)
{
   int success = 0;                               /* we haven't succeeded yet */

   _bmp_read_context ctx;                         /* the read context */

   memset(&ctx, 0, sizeof(_bmp_read_context));    /* must be zeroed */

   do
   {
      /* check for valid output struct */
      if (!p_bmp_out)
      {
         break;
      }

      /* zero it if it exists */
      memset(p_bmp_out, 0, sizeof(glbmp_t));

      /* check for existence of filename */
      if (!bmp_file)
      {
         break;
      }

      /* attempt to open the file */
      if (!(ctx.fp = fopen(bmp_file, "rb")))
      {
         break;
      }

      /* read and validate the bitmap header */
      if (!_bmp_ReadHeader(&ctx))
      {
         break;
      }

      /* read and validate the bitmap info */
      if (!_bmp_ReadInfo(&ctx, flags))
      {
         break;
      }

      /* get ready for decode */
      if (!_bmp_InitDecode(&ctx))
      {
         break;
      }

      /* read and convert bitmap data */
      if (!_bmp_Decode(&ctx, flags))
      {
         break;
      }

      /* we're done, so fill p_bmp_out with info */
      p_bmp_out->width    = (int)ctx.info.width;
      p_bmp_out->height   = ctx.lines;
      p_bmp_out->rgb_data = ctx.rgb_data;

      /* we have succeeded */
      success = 1;
   } while (0);

   /* free everything used in reading (and rgb_data too, if !success) */
   _bmp_FreeContext(&ctx, success);

   return(success);
}


/* see header for details */
void glbmp_FreeBitmap(glbmp_t *p_bmp)
{
   if (p_bmp)                                     /* if valid */
   {
      /* free rgb data */
      if (p_bmp->rgb_data)
      {
         free(p_bmp->rgb_data);
      }

      /* zero it */
      memset(p_bmp, 0, sizeof(glbmp_t));
   }
}


/* _bmp_Swap32 and _bmp_Swap16
 *
 * Performs byte swapping on big-endian architectures (bmp native file format
 * is little-endian).  They both accept a 16 or 32 bit integer and spit back
 * out a byte-swapped representation of that integer.  Word.
 */
#ifdef _GLBMP_BYTESWAP
static uint32_t _bmp_Swap32(uint32_t x)
{
   return((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24));
}


static uint16_t _bmp_Swap16(uint16_t x)
{
   return((x >> 8) | (x << 8));
}


#endif

/* _bmp_ReadHeader
 *
 * Reads the bitmap header from the file in the context object.  Assumes that
 * the file pointer is at the beginning of the file.  Returns 1 if it succeeds
 * or 0 if there's an error (invalid file).
 */
static int _bmp_ReadHeader(_bmp_read_context *p_ctx)
{
   int success = 0;                               /* we haven't succeeded yet */

   do
   {
      /* read first char (should be 'B') */
      if (fgetc(p_ctx->fp) != 0x42)
      {
         break;
      }

      /* read next char (should be 'M') */
      if (fgetc(p_ctx->fp) != 0x4d)
      {
         break;
      }

      /* then read rest of header */
      if (fread(&p_ctx->header, sizeof(_bmp_header), 1, p_ctx->fp) != 1)
      {
         break;
      }

      /* swap bytes for other architectures */
#ifdef _GLBMP_BYTESWAP
      p_ctx->header.file_size   = _bmp_Swap32(p_ctx->header.file_size);
      p_ctx->header.data_offset = _bmp_Swap32(p_ctx->header.data_offset);
#endif

      success = 1;
   } while (0);

   return(success);
}


/* _bmp_IsPowerOf2
 *
 * Returns whether an integer is a power of 2.
 */
static int _bmp_IsPowerOf2(int x)
{
   int bit;                                       /* which bit of the int we're examining */

   /* make sure x is positive */
   if (x < 0)
   {
      x = -x;
   }

   /* for each bit of an int */
   for (bit = 1; bit; bit <<= 1)
   {
      /* if the bit is set */
      if (x & bit)
      {
         return(!(x & ~bit));                     /* return nonzero if no other bits are */
      }
   }

   /* if it didn't find a bit, x was 0, which isn't a power of 2 */
   return(0);
}


/* _bmp_GetLineLength
 *
 * Returns the DWORD-aligned byte-length of a scan line.  For instance, for
 * 24-bit data 3 pixels wide, it would return 12 (3 pixels * 3 bytes each = 9
 * bytes, then padded to the next DWORD).  bpp is BITS per pixel, not bytes.
 */
static int _bmp_GetLineLength(int width, int bpp)
{
   int bits;                                      /* number of bits in a line */
   int pad_bits;                                  /* number of padding bits to make bits divisible by 32 */

   /* set bits */
   bits = width * bpp;

   /* set pad_bits */
   pad_bits = 32 - (bits & 0x1f);                 /* bits & 0x1f == bits % 32, but faster */

   /* if it's not already dword aligned, add pad_bits to it */
   if (pad_bits < 32)
   {
      bits += pad_bits;
   }

   /* return how many bytes we have now */
   return(bits / 8);
}


/* _bmp_ReadInfo
 *
 * Reads and validates the bitmap info struct from the context's file object.
 * Assumes the file pointer is right after the bitmap header.  Returns 1 if ok
 * or 0 if error or invalid file.
 */
static int _bmp_ReadInfo(_bmp_read_context *p_ctx, int flags)
{
   int success = 0;                               /* we haven't succeeded yet */

   do
   {
      /* read in actual info struct */
      if (fread(&p_ctx->info, sizeof(_bmp_info), 1, p_ctx->fp) != 1)
      {
         break;
      }

      /* swap bytes for other architectures */
#ifdef _GLBMP_BYTESWAP
      p_ctx->info.info_size   = _bmp_Swap32(p_ctx->info.info_size);
      p_ctx->info.width       = (int32_t)_bmp_Swap32((uint32_t)p_ctx->info.width);
      p_ctx->info.height      = (int32_t)_bmp_Swap32((uint32_t)p_ctx->info.height);
      p_ctx->info.planes      = _bmp_Swap16(p_ctx->info.planes);
      p_ctx->info.bits        = _bmp_Swap16(p_ctx->info.bits);
      p_ctx->info.compression = _bmp_Swap32(p_ctx->info.compression);
#endif

      /* make sure size is valid (power of 2 checking is later )*/
      if ((p_ctx->info.width <= 0) || (p_ctx->info.height == 0))
      {
         break;
      }

      /* validate compression scheme (none supported yet) */
      if (p_ctx->info.compression > 0)
      {
         break;
      }

      /* validate bit depth */
      if ((p_ctx->info.bits != 1) && (p_ctx->info.bits != 4) &&
          (p_ctx->info.bits != 8) && (p_ctx->info.bits != 24))
      {
         break;
      }

      /* get number of scan lines */
      p_ctx->lines = ((p_ctx->info.height < 0) ?
                      (int)-p_ctx->info.height :
                      (int)p_ctx->info.height);

      /* get length of a scan line (it's dword aligned in the file) */
      p_ctx->file_line_len = _bmp_GetLineLength(p_ctx->info.width,
                                                p_ctx->info.bits);

      /* get length of scan line in output buffer (either aligned or not) */
      p_ctx->rgb_line_len = ((flags & GLBMP_BYTE_ALIGN) ?
                             (int)p_ctx->info.width * 3 :
                             _bmp_GetLineLength(p_ctx->info.width, 24));

      /* if we're only allowing power of two sized images */
      if (!(flags & GLBMP_ANY_SIZE))
      {
         /* make sure width and height are both powers of two */
         if (!_bmp_IsPowerOf2((int)p_ctx->info.width))
         {
            break;
         }
         if (!_bmp_IsPowerOf2(p_ctx->lines))
         {
            break;
         }
      }

      /* if it has a palette */
      if (p_ctx->info.bits <= 8)
      {
         int colors = 1 << p_ctx->info.bits;      /* how many colors */

         /* alloc space for palette */
         if (!(p_ctx->palette = (_bmp_palette_entry *)
                                malloc(colors * sizeof(_bmp_palette_entry))))
         {
            break;
         }

         /* go to the palette in the file */
         if (fseek(p_ctx->fp,
                   p_ctx->info.info_size - sizeof(_bmp_info), SEEK_CUR))
         {
            break;
         }

         /* read the palette */
         if (fread(p_ctx->palette, sizeof(_bmp_palette_entry),
                   colors, p_ctx->fp) != colors)
         {
            break;
         }
      }

      success = 1;
   } while (0);

   return(success);
}


/* _bmp_InitDecode
 *
 * Allocates memory for the decode operation and sets up the file pointer.
 * Returns 1 if success, 0 if failure.
 */
static int _bmp_InitDecode(_bmp_read_context *p_ctx)
{
   /* alloc a line of file data */
   return((p_ctx->file_data = (uint8_t *)malloc(p_ctx->file_line_len))
          /* alloc the entire output buffer */
          && (p_ctx->rgb_data = (uint8_t *)
                                malloc(p_ctx->rgb_line_len * p_ctx->lines))
          /* go to beginning of bitmap data in file */
          && !fseek(p_ctx->fp, p_ctx->header.data_offset, SEEK_SET));
}


/* _bmp_Decode24
 *
 * Decodes 24-bit bitmap data.  Takes a pointer to an output buffer scan line
 * (p_rgb), a pointer to the end of the PIXEL DATA of this scan line
 * (p_rgb_end), a pointer to the source scan line of file data (p_file), and a
 * pointer to a palette (not used in this case).
 */
static void _bmp_Decode24(uint8_t *p_rgb, uint8_t *p_rgb_end,
                          uint8_t *p_file, _bmp_palette_entry *palette)
{
   /* for each pixel */
   while (p_rgb < p_rgb_end)
   {
      *p_rgb++ = *(p_file++ + 2);                 /* red is two file bytes ahead */
      *p_rgb++ = *(p_file++);                     /* green is right here */
      *p_rgb++ = *(p_file++ - 2);                 /* blue is now two bytes back */
   }
}


/* _bmp_Decode8
 *
 * Same as _bmp_Decode24, but for 8 bit palette data.
 */
static void _bmp_Decode8(uint8_t *p_rgb, uint8_t *p_rgb_end,
                         uint8_t *p_file, _bmp_palette_entry *palette)
{
   /* for each pixel */
   while (p_rgb < p_rgb_end)
   {
      *p_rgb++ = palette[*p_file].red;            /* get each color */
      *p_rgb++ = palette[*p_file].green;          /* component from */
      *p_rgb++ = palette[*p_file++].blue;         /* the palette    */
   }
}


/* _bmp_Decode4
 *
 * Same as _bmp_Decode24, but for 4 bit palette data.
 */
static void _bmp_Decode4(uint8_t *p_rgb, uint8_t *p_rgb_end,
                         uint8_t *p_file, _bmp_palette_entry *palette)
{
   int lookup;                                    /* pixel value for palette */

   /* for each pixel */
   while (p_rgb < p_rgb_end)
   {
      lookup = (*p_file & 0xf0) >> 4;             /* get palette lookup */

      *p_rgb++ = palette[lookup].red;             /* get each color */
      *p_rgb++ = palette[lookup].green;           /* component from */
      *p_rgb++ = palette[lookup].blue;            /* the palette    */

      if (p_rgb < p_rgb_end)
      {
         lookup = *p_file++ & 0x0f;               /* get palette lookup */

         *p_rgb++ = palette[lookup].red;          /* get each color */
         *p_rgb++ = palette[lookup].green;        /* component from */
         *p_rgb++ = palette[lookup].blue;         /* the palette    */
      }
   }
}


/* _bmp_Decode1
 *
 * Same as _bmp_Decode24, but for monochrome palette data.
 */
static void _bmp_Decode1(uint8_t *p_rgb, uint8_t *p_rgb_end,
                         uint8_t *p_file, _bmp_palette_entry *palette)
{
   int shift;                                     /* how far to rshift current byte to get current bit */
   int lookup;                                    /* pixel value for palette */

   while (p_rgb < p_rgb_end)
   {
      /* check every bit in byte */
      for (shift = 7; shift >= 0 && p_rgb < p_rgb_end; --shift)
      {
         lookup = (*p_file >> shift) & 1;         /* get palette lookup */

         *p_rgb++ = palette[lookup].red;          /* get each color */
         *p_rgb++ = palette[lookup].green;        /* component from */
         *p_rgb++ = palette[lookup].blue;         /* the palette    */
      }

      p_file++;                                   /* go to next byte */
   }
}


/* _bmp_Decode
 *
 * Selects an above decoder and runs it for each scan line of the file.
 * Returns 0 if there's an error or 1 if it's gravy.
 */
static int _bmp_Decode(_bmp_read_context *p_ctx, int flags)
{
   /* decoder function */
   void (*decoder)(uint8_t *, uint8_t *, uint8_t *,
                   _bmp_palette_entry *) = NULL;

   uint8_t *p_rgb;                                /* pointer to current scan line in output buffer */
   int     rgb_inc;                               /* incrementor for scan line in outupt buffer */
   uint8_t *p_rgb_end;                            /* end marker for output buffer */
   uint8_t *p_line_end;                           /* pointer to end of current scan line in output */

   /* select decoder function based on bit depth of source */
   switch (p_ctx->info.bits)
   {
   case 24:
      decoder = _bmp_Decode24;
      break;

   case 8:
      decoder = _bmp_Decode8;
      break;

   case 4:
      decoder = _bmp_Decode4;
      break;

   case 1:
      decoder = _bmp_Decode1;
      break;
   }
   /* if compressed, use that one instead (not implemented yet) */

   /*
    * switch(p_ctx->info.compression)
    * {
    * case 1: decoder = _bmp_DecodeRle8; break;
    * case 2: decoder = _bmp_DecodeRle4; break;
    * }
    */

   /* if we don't need to reverse the order of scan lines */
   if (!(p_ctx->info.height < 0) == !(flags & GLBMP_TOP_DOWN))
   {
      /* start our decoder at the beginning of output buffer */
      p_rgb = p_ctx->rgb_data;
      /* go forward one line length each iteration */
      rgb_inc = p_ctx->rgb_line_len;
      /* end marker is line after end of actual output buffer */
      p_rgb_end = p_ctx->rgb_data + (p_ctx->rgb_line_len * p_ctx->lines);
   }
   else                                           /* we DO need to reverse order of scan lines */
   {
      /* start the decoder at the last line of output buffer */
      p_rgb = p_ctx->rgb_data + (p_ctx->rgb_line_len * (p_ctx->lines - 1));
      /* go backward one line length each iteration */
      rgb_inc = -p_ctx->rgb_line_len;
      /* end marker is the line BEFORE START of output buffer */
      p_rgb_end = p_ctx->rgb_data - p_ctx->rgb_line_len;
   }

   /* set end of line to be whatever we decided the beginning of the line is */
   /* plus number of pixels in line * number of bytes per pixel              */
   p_line_end = p_rgb + p_ctx->info.width * 3;

   if (decoder)                                   /* if we found a decoder */
   {
      /* iterate until we reach the end of our buffers, or we have a read error */
      while (p_rgb != p_rgb_end &&
             fread(p_ctx->file_data, p_ctx->file_line_len, 1, p_ctx->fp) == 1)
      {
         /* decode the line */
         decoder(p_rgb, p_line_end, p_ctx->file_data, p_ctx->palette);
         p_rgb      += rgb_inc;                   /* update line pointers */
         p_line_end += rgb_inc;
      }
   }

   /* if we reached the end of the buffer, we succeeded */
   return(p_rgb == p_rgb_end);
}


/* _bmp_FreeContext
 *
 * Frees resources allocated by various functions along the way.  Only frees
 * rgb_data if !leave_rgb_data (if the bitmap loads successfully, you want the
 * data to remain until THEY free it).
 */
static void _bmp_FreeContext(_bmp_read_context *p_ctx, int leave_rgb_data)
{
   /* free/close everything */
   if (p_ctx->fp)
   {
      fclose(p_ctx->fp);
   }
   if (p_ctx->palette)
   {
      free(p_ctx->palette);
   }
   if (p_ctx->file_data)
   {
      free(p_ctx->file_data);
   }

   /* free rgb_data only if we had an error */
   if (!leave_rgb_data && p_ctx->rgb_data)
   {
      free(p_ctx->rgb_data);
   }
}
