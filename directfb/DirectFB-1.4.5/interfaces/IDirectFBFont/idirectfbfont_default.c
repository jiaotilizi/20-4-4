/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <stdarg.h>

#include <directfb.h>

#include <core/fonts.h>
#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>

#include <gfx/convert.h>

#include <media/idirectfbfont.h>

#include <direct/hash.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/utf8.h>

#include "default_font.h"

#define DEFAULT_FONT_HEIGHT     24
#define DEFAULT_FONT_ASCENDER   16
#define DEFAULT_FONT_DESCENDER  -4


static DFBResult
Probe( IDirectFBFont_ProbeContext *ctx );

static DFBResult
Construct( IDirectFBFont               *thiz,
           CoreDFB                     *core,
           IDirectFBFont_ProbeContext  *ctx,
           DFBFontDescription          *desc );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBFont, Default )


static DFBResult
Probe( IDirectFBFont_ProbeContext *ctx )
{
#ifdef TIVO
    if (ctx->filename)
    {
        const char* useDefaultFont;
        const char* fontFile;
        const char* fakeFontFile;

        // see if the default font renderer is forced turned on
        useDefaultFont = getenv( "DFB_USE_DEFAULT_FONT" );
        if ((useDefaultFont != NULL) &&
            (strcmp( useDefaultFont, "1" ) == 0))
        {
            return DFB_OK;
        }

        // extract the font file name from the path
        fontFile = strrchr( ctx->filename, '/' );
        if (fontFile == NULL)
        {
            fontFile = ctx->filename;
        }
        else
        {
            fontFile++;
        }

        // allow passing a dummy file name to activate it too
        if (strcmp( fontFile, "dfb-default-font" ) == 0)
        {
            return DFB_OK;
        }

        // lastly allow it to be activated for a specific font
        fakeFontFile = getenv( "DFB_FAKE_FONT_FILE" );
        if ((fakeFontFile != NULL) &&
            (strcmp( fakeFontFile, fontFile ) == 0))
        {
            return DFB_OK;
        }
    }
#endif

     /* default font is created with a NULL filename */
     if (ctx->filename)
          return DFB_UNSUPPORTED;

     return DFB_OK;
}

static DFBResult
Construct( IDirectFBFont               *thiz,
           CoreDFB                     *core,
           IDirectFBFont_ProbeContext  *ctx,
           DFBFontDescription          *desc )
{
     DFBResult         ret;
     CoreFont         *font;
     CoreSurface      *surface;
     CoreFontCacheRow *row;
     u8               *pixels;
     int               i;

     CoreSurfaceConfig  config;

     D_DEBUG( "DirectFB/FontDefault: Construct default font");

     ret = dfb_font_create( core, &font );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     D_ASSERT( font->pixel_format == DSPF_ARGB ||
               font->pixel_format == DSPF_AiRGB ||
               font->pixel_format == DSPF_ARGB8565 ||
               font->pixel_format == DSPF_ARGB4444 ||
               font->pixel_format == DSPF_RGBA4444 ||
               font->pixel_format == DSPF_ARGB2554 ||
               font->pixel_format == DSPF_ARGB1555 ||
               font->pixel_format == DSPF_RGBA5551 ||
               font->pixel_format == DSPF_A8 ||
               font->pixel_format == DSPF_A4 ||
               font->pixel_format == DSPF_A1 ||
               font->pixel_format == DSPF_A1_LSB );

     font->height    = DEFAULT_FONT_HEIGHT;
     font->ascender  = DEFAULT_FONT_ASCENDER;
     font->descender = DEFAULT_FONT_DESCENDER;

     row = D_CALLOC( 1, sizeof(CoreFontCacheRow) );
     if (!row) {
          D_OOM();
          dfb_font_destroy( font );
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return DFB_NOSYSTEMMEMORY;
     }

     config.flags  = CSCONF_SIZE | CSCONF_FORMAT | CSCONF_CAPS;
     config.size.w = font_desc.width;
     config.size.h = font_desc.height;
     config.format = font->pixel_format;
     config.caps   = font->surface_caps;

     ret = dfb_surface_create( core, &config, CSTF_FONT, 0, NULL, &surface );
     if (ret) {
          dfb_font_destroy( font );
          return ret;
     }

     font->num_rows  = 1;
     font->row_width = font_desc.width;
     font->rows      = D_MALLOC(sizeof (void *));
     font->rows[0]   = row;

     row->surface = surface;

     D_MAGIC_SET( row, CoreFontCacheRow );

     pixels = font_data;

     {
          CoreGlyphData *data;
          int use_unicode;
          int start = 0;
          int index = 0;
          int key;
          const char *glyphs =
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz"
          "01234567890!\"$%&/()=?^<>" // FIXME: '0' is repeated!
          "|,;.:-_{[]}\\`+*~#'";

          if (desc && (desc->flags & DFDESC_ATTRIBUTES) &&
              (desc->attributes & DFFA_NOCHARMAP))
               use_unicode = 0;
          else
               use_unicode = 1;

          for (i = 0; i < font_desc.width; i++) {
               if (pixels[i] == 0xFF) {
                    if (use_unicode)
                         key = glyphs[index];
                    else
                         key = index;
                         
                    if (!direct_hash_lookup( font->layers[0].glyph_hash, key )) { 
                         data = D_CALLOC( 1, sizeof(CoreGlyphData) );
                         data->surface = surface;
                         data->start   = start;
                         data->width   = i - start + 1;
                         data->height  = font_desc.height - 1;
                         data->left    = 0;
                         data->top     = 0;
                         data->advance = ((desc && (desc->flags &
                                                    DFDESC_FIXEDADVANCE)) ?
                                         desc->fixed_advance :
                                         data->width + 1);

                         D_DEBUG( "DirectFB/core/fonts: "
                                       "glyph '%c' at %d, width %d\n",
                                       glyphs[index], start, i-start );

                         D_MAGIC_SET( data, CoreGlyphData );

                         if (font->maxadvance < data->advance)
                              font->maxadvance = data->advance;
                              
                         direct_hash_insert( font->layers[0].glyph_hash, key, data );
                    }

                    start = i + 1;
                    index++;
               }
               if (glyphs[index] == '\0')
                    break;
          }

          /*  space  */
          data = D_CALLOC( 1, sizeof(CoreGlyphData) );
          data->advance = 5;

          D_MAGIC_SET( data, CoreGlyphData );

          if (use_unicode)
               key = 32;
          else
               key = index;

          direct_hash_insert( font->layers[0].glyph_hash, key, data );
     }

     {
          CoreSurfaceBufferLock lock;

          ret = dfb_surface_lock_buffer( surface, CSBR_BACK, CSAID_CPU, CSAF_WRITE, &lock );
          if (ret) {
               D_DERROR( ret, "IDirectFBFont_Default: Could not lock surface buffer!\n" );
          }
          else {
               for (i = 1; i < font_desc.height; i++) {
                    int    i, j, n;
                    u8  *dst8  = lock.addr;
                    u16 *dst16 = lock.addr;
                    u32 *dst32 = lock.addr;

                    pixels += font_desc.preallocated[0].pitch;
                    switch (surface->config.format) {
                         case DSPF_ARGB:
                              if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                                   for (i=0; i<font_desc.width; i++)
                                        dst32[i] = pixels[i] * 0x01010101;
                              }
                              else
                                   for (i=0; i<font_desc.width; i++)
                                        dst32[i] = (pixels[i] << 24) | 0xFFFFFF;
                              break;
                         case DSPF_AiRGB:
                              for (i=0; i<font_desc.width; i++)
                                   dst32[i] = ((pixels[i] ^ 0xFF) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_ARGB8565:
                              for (n = 0, j = -1; n < font_desc.width; ++n) {
                                   u32 d;
                                   if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                                        d = (pixels[n] << 16) * 0x01010101;
                                        d = ARGB_TO_ARGB8565 (d);
                                   }
                                   else
                                        d = (pixels[n] << 16) | 0xFFFF;
#ifdef WORDS_BIGENDIAN
                                   dst8[++j] = (d >> 16) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >>  0) & 0xff;
#else
                                   dst8[++j] = (d >>  0) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >> 16) & 0xff;
#endif
                              }
                              break;
                         case DSPF_ARGB4444:
                         case DSPF_RGBA4444:
                              if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                                   for (i=0; i<font_desc.width; i++)
                                        dst16[i] = (pixels[i] >> 4) * 0x1111;
                              }
                              else {
                                   if( surface->config.format == DSPF_ARGB4444 ) {
                                        for (i=0; i<font_desc.width; i++)
                                             dst16[i] = (pixels[i] << 8) | 0x0FFF;
                                   } else {
                                        for (i=0; i<font_desc.width; i++)
                                             dst16[i] = (pixels[i] >> 4) | 0xFFF0;
                                   }
                              }
                              break;
                         case DSPF_ARGB2554:
                              for (i=0; i<font_desc.width; i++)
                                   dst16[i] = (pixels[i] << 8) | 0x3FFF;
                              break;
                         case DSPF_ARGB1555:
                              if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                                   for (i=0; i<font_desc.width; i++) {
                                        unsigned short x = pixels[i] >> 3;
                                        dst16[i] = ((pixels[i] & 0x80) << 8) |
                                             (x << 10) | (x << 5) | x;
                                   }
                              }
                              else {
                                   for (i=0; i<font_desc.width; i++)
                                        dst16[i] = (pixels[i] << 8) | 0x7FFF;
                              }
                              break;
                         case DSPF_RGBA5551:
                              if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                                   for (i=0; i<font_desc.width; i++) {
                                        unsigned short x = pixels[i] >> 3;
                                        dst16[i] = (x << 11) | (x << 6) | (x << 1) |
                                             (pixels[i] >> 7);
                                   }
                              }
                              else {
                                   for (i=0; i<font_desc.width; i++)
                                        dst16[i] = 0xFFFE | (pixels[i] >> 7);
                              }
                              break;
                         case DSPF_A8:
                              direct_memcpy( lock.addr, pixels, font_desc.width );
                              break;
                         case DSPF_A4:
                              for (i=0, j=0; i<font_desc.width; i+=2, j++)
                                   dst8[j] = (pixels[i] & 0xF0) | (pixels[i+1] >> 4);
                              break;
                         case DSPF_A1:
                              for (i=0, j=0; i < font_desc.width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<font_desc.width; ++i, ++n)
                                        p |= (pixels[i] & 0x80) >> n;

                                   dst8[j] = p;
                              }
                              break;
                         case DSPF_A1_LSB:
                              for (i=0, j=0; i < font_desc.width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<font_desc.width; ++i, ++n)
                                        p |= (pixels[i] & 0x80) >> (7-n);

                                   dst8[j] = p;
                              }
                              break;
                         case DSPF_LUT2:
                              for (i=0, j=0; i < font_desc.width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<font_desc.width; ++i, n+=2)
                                        p |= (pixels[i] & 0xC0) >> n;

                                   dst8[j] = p;
                              }
                              break;
                         default:
                              D_UNIMPLEMENTED();
                              break;
                    }

                    lock.addr += lock.pitch;
               }

               dfb_surface_unlock_buffer( surface, &lock );
          }
     }

     return IDirectFBFont_Construct (thiz, font);
}
