/***************************************************************************
 *   Copyright (C) 2020 by Federico Izzo IU2NUO,                           *
 *                         Niccolò Izzo IU2KIN,                            *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

/**
 * This source file provides an  implementation for the graphics.h interface
 * It is suitable for both color, grayscale and B/W display
 */

#include <os.h>
#include <string.h>
#include <stdio.h>
#include <hwconfig.h>
#include <interfaces/display.h>
#include <interfaces/graphics.h>

#ifdef PIX_FMT_RGB565
/* This specialization is meant for an RGB565 little endian pixel format.
 * Thus, to accomodate for the endianness, the fields in struct rgb565_t have to
 * be written in reversed order.
 *
 * For more details about endianness and bitfield structs see the following web
 * page: http://mjfrazer.org/mjfrazer/bitfields/
 */

typedef struct
{
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;
} rgb565_t;

rgb565_t _true2highColor(color_t true_color)
{
    rgb565_t high_color;
    high_color.r = true_color.r >> 3;
    high_color.g = true_color.g >> 2;
    high_color.b = true_color.b >> 3;

    return high_color;
}

#define PIXEL_T rgb565_t
#elif defined PIX_FMT_BW
/**
 * This specialization is meant for black and white pixel format.
 * It is suitable for monochromatic displays with 1 bit per pixel,
 * it will have RGB and grayscale counterparts
 */

typedef enum
{
    WHITE = 0,
    BLACK = 1,
} bw_t;

bw_t _color2bw(color_t true_color)
{
    if(true_color.r == 0 &&
       true_color.g == 0 &&
       true_color.b == 0)
        return WHITE;
    else
        return BLACK;
}

#define PIXEL_T uint8_t
#else
#error Please define a pixel format type into hwconfig.h or meson.build
#endif

bool initialized = 0;
PIXEL_T *buf;
uint16_t fbSize;

void gfx_init()
{
    display_init();
    buf = (PIXEL_T *)(display_getFrameBuffer());
    initialized = 1;

// Calculate framebuffer size
#ifdef PIX_FMT_RGB565
    fbSize = SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(PIXEL_T);
#elif defined PIX_FMT_BW
    fbSize = (SCREEN_HEIGHT * SCREEN_WIDTH) / 8;
    /* Compensate for eventual truncation error in division */
    if((fbSize * 8) < (SCREEN_HEIGHT * SCREEN_WIDTH)) fbSize += 1;
    fbSize *= sizeof(uint8_t);
#endif
}

void gfx_terminate()
{
    display_terminate();
    initialized = 0;
}

void gfx_renderRows(uint8_t startRow, uint8_t endRow)
{
    display_renderRows(startRow, endRow);
}

void gfx_render()
{
    display_render();
}

bool gfx_renderingInProgress()
{
    return display_renderingInProgress();
}

void gfx_clearRows(uint8_t startRow, uint8_t endRow)
{
    if(!initialized) return;
    if(endRow < startRow) return;
    uint16_t start = startRow * SCREEN_WIDTH * sizeof(PIXEL_T);
    uint16_t height = endRow - startRow * SCREEN_WIDTH * sizeof(PIXEL_T);
    // Set the specified rows to 0x00 = make the screen black
    memset(buf + start, 0x00, height);
}

void gfx_clearScreen()
{
    if(!initialized) return;
    // Set the whole framebuffer to 0x00 = make the screen black
    memset(buf, 0x00, fbSize);
}

void gfx_fillScreen(color_t color)
{
    if(!initialized) return;
    for(int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for(int x = 0; x < SCREEN_WIDTH; x++)
        {
            point_t pos = {x, y};
            gfx_setPixel(pos, color);
        }
    }
}

inline void gfx_setPixel(point_t pos, color_t color)
{
    if (pos.x >= SCREEN_WIDTH || pos.y >= SCREEN_HEIGHT)
        return; // off the screen

#ifdef PIX_FMT_RGB565
    // Blend old pixel value and new one
    if (color.alpha < 255)
    {
        uint8_t alpha = color.alpha;
        rgb565_t new_pixel = _true2highColor(color);
        rgb565_t old_pixel = buf[pos.x + pos.y*SCREEN_WIDTH];
        rgb565_t pixel = {((255-alpha)*old_pixel.b+alpha*new_pixel.b)/255,
                          ((255-alpha)*old_pixel.g+alpha*new_pixel.g)/255,
                          ((255-alpha)*old_pixel.r+alpha*new_pixel.r)/255};
        buf[pos.x + pos.y*SCREEN_WIDTH] = pixel;
    }
    else
    {
        buf[pos.x + pos.y*SCREEN_WIDTH] = _true2highColor(color);
    }
#elif defined PIX_FMT_BW
    // Ignore more than half transparent pixels
    if (color.alpha >= 128)
    {
        uint16_t cell = (pos.x + pos.y*SCREEN_WIDTH) / 8;
        uint16_t elem = (pos.x + pos.y*SCREEN_WIDTH) % 8;
        buf[cell] &= ~(1 << elem);
        buf[cell] |= (_color2bw(color) << elem);
    }
#endif
}

void gfx_drawLine(point_t start, point_t end, color_t color)
{
    if(!initialized) return;
    for(int y = start.y; y <= end.y; y++)
    {
        for(int x = start.x; x <= end.x; x++)
        {
            point_t pos = {x, y};
            gfx_setPixel(pos, color);
        }
    }
}

void gfx_drawRect(point_t start, uint16_t width, uint16_t height, color_t color, bool fill)
{
    if(!initialized) return;
    if(width == 0) return;
    if(height == 0) return;
    uint16_t x_max = start.x + width - 1;
    uint16_t y_max = start.y + height - 1;
    bool perimeter = 0;
    if(x_max > (SCREEN_WIDTH - 1)) x_max = SCREEN_WIDTH - 1;
    if(y_max > (SCREEN_HEIGHT - 1)) y_max = SCREEN_HEIGHT - 1;
    for(int y = start.y; y <= y_max; y++)
    {
        for(int x = start.x; x <= x_max; x++)
        {
            if(y == start.y || y == y_max || x == start.x || x == x_max) perimeter = 1;
            else perimeter = 0;
            // If fill is false, draw only rectangle perimeter
            if(fill || perimeter)
            {
                point_t pos = {x, y};
                gfx_setPixel(pos, color);
            }
        }
    }
}

void gfx_drawHLine(uint16_t y, uint16_t height, color_t color)
{
    point_t start = {0, y};
    gfx_drawRect(start, SCREEN_WIDTH, height, color, 1);
}

void gfx_drawVLine(uint16_t x, uint16_t width, color_t color)
{
    point_t start = {x, 0};
    gfx_drawRect(start, width, SCREEN_HEIGHT, color, 1);
}

/**
 * Compute the pixel size of the first text line
 * @param f: font used as the source of glyphs
 * @param text: the input text
 * @param length: the length of the input text, used for boundary checking
 */
static inline uint16_t get_line_size(GFXfont f, const char *text, uint16_t length)
{
    uint16_t line_size = 0;
    for(unsigned i = 0; i < length && text[i] != '\n' && text[i] != '\r'; i++)
    {
        GFXglyph glyph = f.glyph[text[i] - f.first];
        if (line_size + glyph.xAdvance < SCREEN_WIDTH)
            line_size += glyph.xAdvance;
        else
            break;
    }

    return line_size;
}

/**
 * Compute the start x coordinate of a new line of given pixel size
 * @param alinment: enum representing the text alignment
 * @param line_size: the size of the current text line in pixels
 */
static inline uint16_t get_reset_x(textAlign_t alignment, uint16_t line_size,
                                                          uint16_t startx)
{
    switch(alignment)
    {
        case TEXT_ALIGN_LEFT:
            return startx;
        case TEXT_ALIGN_CENTER:
            return (SCREEN_WIDTH - line_size)/2;
        case TEXT_ALIGN_RIGHT:
            return SCREEN_WIDTH - line_size - startx;
    }

    return 0;
}

point_t gfx_print(point_t start, const char *text, fontSize_t size,
                                 textAlign_t alignment, color_t color)
{

    GFXfont f = fonts[size];

    size_t len = strlen(text);

    // Compute size of the first row in pixels
    uint16_t line_size = get_line_size(f, text, len);
    uint16_t reset_x = get_reset_x(alignment, line_size, start.x);
    start.x = reset_x;

    // Save initial start.y value to calculate vertical size
    uint16_t saved_start_y = start.y;
    uint16_t line_h = 0;

    /* For each char in the string */
    for(unsigned i = 0; i < len; i++)
    {
        char c = text[i];
        GFXglyph glyph = f.glyph[c - f.first];
        uint8_t *bitmap = f.bitmap;

        uint16_t bo = glyph.bitmapOffset;
        uint8_t w = glyph.width, h = glyph.height;
        int8_t xo = glyph.xOffset,
               yo = glyph.yOffset;
        uint8_t xx, yy, bits = 0, bit = 0;
        line_h = h;

        // Handle newline and carriage return
        if (c == '\n')
        {
          start.x = reset_x;
          start.y += f.yAdvance;
          continue;
        }
        else if (c == '\r')
        {
          start.x = reset_x;
          continue;
        }

        // Handle wrap around
        if (start.x + glyph.xAdvance > SCREEN_WIDTH)
        {
            // Compute size of the first row in pixels
            line_size = get_line_size(f, text, len);
            start.x = reset_x = get_reset_x(alignment, line_size, start.x);
            start.y += f.yAdvance;
        }

        // Draw bitmap
        for (yy = 0; yy < h; yy++)
        {
            for (xx = 0; xx < w; xx++)
            {
                if (!(bit++ & 7))
                {
                    bits = bitmap[bo++];
                }

                if (bits & 0x80)
                {
                    if (start.y + yo + yy < SCREEN_HEIGHT &&
                        start.x + xo + xx < SCREEN_WIDTH &&
                        start.y + yo + yy > 0 &&
                        start.x + xo + xx > 0)
                    {
                        point_t pos = {start.x + xo + xx, start.y + yo + yy};
                        gfx_setPixel(pos, color);

                    }
                }

                bits <<= 1;
            }
        }

        start.x += glyph.xAdvance;
    }
    // Calculate text size
    point_t text_size = {0, 0};
    text_size.x = line_size;
    text_size.y = (saved_start_y - start.y) + line_h;
    return text_size;
}

// Print an error message to the center of the screen, surronded by a red (when possible) box
void gfx_printError(const char *text, fontSize_t size)
{
    // 3 px box padding
    uint16_t box_padding = 16;
    color_t white = {255, 255, 255, 255};
    color_t red =   {255,   0,   0, 255};
    point_t start = {0, SCREEN_HEIGHT/2 + 5};

    // Print the error message
    point_t text_size = gfx_print(start, text, size, TEXT_ALIGN_CENTER, white);
    text_size.x += box_padding;
    text_size.y += box_padding;
    point_t box_start = {0, 0};
    box_start.x = (SCREEN_WIDTH / 2) - (text_size.x / 2);
    box_start.y = (SCREEN_HEIGHT / 2) - (text_size.y / 2);
    // Draw the error box
    gfx_drawRect(box_start, text_size.x, text_size.y, red, false);
}

/*
 * Function to draw battery of arbitrary size
 * starting coordinates are relative to the top left point.
 *
 *  ****************       |
 * *                *      |
 * *  *******       *      |
 * *  *******       **     |
 * *  *******       **     | <-- Height (px)
 * *  *******       *      |
 * *                *      |
 *  ****************       |
 *
 * __________________
 *
 * ^
 * |
 *
 * Width (px)
 *
 */
void gfx_drawBattery(point_t start, uint16_t width, uint16_t height,
                                                    float percentage)
{
    color_t white =  {255, 255, 255, 255};
    color_t black =  {0,   0,   0  , 255};

    // Cap percentage to 1
    percentage = (percentage > 1.0f) ? 1.0f : percentage;

#ifdef PIX_FMT_RGB565
    color_t green =  {0,   255, 0  , 255};
    color_t yellow = {250, 180, 19 , 255};
    color_t red =    {255, 0,   0  , 255};

    // Select color according to percentage
    color_t bat_color = yellow;
    if (percentage < 0.3)
        bat_color = red;
    else if (percentage > 0.6)
        bat_color = green;
#elif defined PIX_FMT_BW
    color_t bat_color = white;
#endif

    // Draw the battery outline
    gfx_drawRect(start, width, height, white, false);

    // Draw the battery fill
    point_t fill_start = {start.x + 2, start.y + 2};
    gfx_drawRect(fill_start, (int)(((float)(width - 4)) * percentage),
                 height - 4, bat_color, true);

    // Round corners
    point_t top_left = start;
    point_t top_right = {start.x + width - 1, start.y};
    point_t bottom_left = {start.x, start.y + height - 1};
    point_t bottom_right = {start.x + width - 1, start.y + height - 1};
    gfx_setPixel(top_left, black);
    gfx_setPixel(top_right, black);
    gfx_setPixel(bottom_left, black);
    gfx_setPixel(bottom_right, black);

    // Draw the button
    point_t button_start = {start.x + width, start.y + height / 2 - (height / 8) - 1 + (height % 2)};
    point_t button_end =   {start.x + width, start.y + height / 2 + (height / 8)};
    gfx_drawLine(button_start, button_end, white);
}

/*
 * Function to draw RSSI-meter of arbitrary size
 * starting coordinates are relative to the top left point.
 *
 * *         *         *         *           *          *           *|
 * *****************************************                         |
 * ******************************************    <- RSSI             |
 * ******************************************                        |  <-- Height (px)
 * ******************************************                        |
 * ****************             <-- Squelch                          |
 * ***************                                                   |
 * *         *         *         *           *          *           *|
 * ___________________________________________________________________
 *
 * ^
 * |
 *
 * Width (px)
 *
 */
void gfx_drawSmeter(point_t start, uint16_t width, uint16_t height, float rssi,
                                                   float squelch, color_t color)
{
    color_t white =  {255, 255, 255, 255};
    color_t yellow = {250, 180, 19 , 255};
    color_t red =    {255, 0,   0  , 255};

    // S-level dots
    for(int i = 0; i < 11; i++)
    {
        color_t color = (i % 3 == 0) ? yellow : white;
        color = (i > 9) ? red : color;
        point_t pixel_pos = {start.x + i * (width - 1) / 11, start.y};
        gfx_setPixel(pixel_pos, color);
        pixel_pos.y += height;
        gfx_setPixel(pixel_pos, color);
    }

    point_t pixel_pos = {start.x + width - 1, start.y};
    gfx_setPixel(pixel_pos, red);
    pixel_pos.y += height;
    gfx_setPixel(pixel_pos, red);

    // RSSI bar
    uint16_t rssi_height = height * 2 / 3;
    float s_level =  (127.0f + rssi) / 6.0f;
    uint16_t rssi_width = (s_level < 0.0f) ? 0 : (s_level * (width - 1) / 11);
    rssi_width = (s_level > 10.0f) ? width : rssi_width;
    point_t rssi_pos = { start.x, start.y + 1 };
    gfx_drawRect(rssi_pos, rssi_width, rssi_height, white, true);

    // Squelch bar
    uint16_t squelch_height = height / 3 - 1;
    uint16_t squelch_width = width * squelch;
    point_t squelch_pos = { start.x, start.y + 1 + rssi_height };
    gfx_drawRect(squelch_pos, squelch_width, squelch_height, color, true);
}
