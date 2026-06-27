#include <stdint.h>

#include "color.h"

struct RGB {
    uint8_t r;  // 0-255
    uint8_t g;  // 0-255
    uint8_t b;  // 0-255
};  // 3 bytes

struct RGBA {
    uint8_t r;  // 0-255
    uint8_t g;  // 0-255
    uint8_t b;  // 0-255
    uint8_t a;  // 0-255, 0=transparent 255=opaque
};  // 4 bytes

class FrameBuffer {
    private:

    uint32_t get_color_hex(Color color){
        switch (color){
            case Color::BLACK: return 0x00000000;
            case Color::WHITE: return 0xFFFFFFFF;
            case Color::RED: return 0x00FF0000;
            default: return 0x00000000;
        }
    }

    uint8_t* glyphs;
    uint32_t width;
    uint32_t height;
    uint64_t base;
    uint32_t pitch;

    public:

    FrameBuffer(uint32_t w, uint32_t h, uint64_t b, uint32_t p) : width(w), height(h), base(b), pitch(p) {}
    
    void set_font(uint8_t address[]){
        glyphs = address + 4;
    }

    uint32_t get_width(){
        return width;
    }

    uint32_t get_height(){
        return height;
    }


    void draw_string(const char* str, uint32_t start_x, uint32_t start_y, Color color) {
        uint32_t cursor_x = start_x, cursor_y = start_y;

        for (uint32_t i = 0; str[i] != '\0'; i++) {
            uint8_t* glyph = get_glyph((uint8_t)str[i]);

            for (int row = 0; row < 16; row++) {
                uint8_t bits = glyph[row];
                for (int col = 0; col < 8; col++) {
                    if (bits & (0x80 >> col)) {
                        uint32_t px = cursor_x * 8 + col;
                        uint32_t py = cursor_y * 16 + row;
                        uint32_t* pixel = (uint32_t*)(base + py * pitch + px * 4);
                        *pixel = get_color_hex(color);
                    }
                }
            }

            cursor_x++;
            if (cursor_x * 8 >= width) {
                cursor_x = start_x;
                cursor_y++;
            }
        }
    }

    void draw_image(RGB* image, uint32_t x, uint32_t y, uint32_t w, uint32_t h){
        for (uint32_t row = 0; row < h; row++) {
            for (uint32_t col = 0; col < w; col++) {
                const RGB& px = image[row * w + col];
                set_pixel(x + col, y + row, px.r, px.g, px.b);  // was just x, y
            }
        }
    }

    void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color){
        for (uint32_t row = x; row < w + x; row++){
            for (uint32_t col = y; col < h + y; col++){
                draw_pixel(row, col, color);
            }
        }
    }

    void draw_square(int32_t x, uint32_t y, uint32_t len, Color color){
        for (uint32_t row = x; row < len + x; row++){
            for (uint32_t col = y; col < len + y; col++){
                draw_pixel(row, col, color);
            }
        }
    }

    void set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
        uint32_t* pixel = (uint32_t*)(base + y * pitch + x * 4);
        *pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    // this is not a proper algo

    void draw_line(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color){
        draw_rectangle(x, y, w, h, color);        
    }

    void draw_pixel(uint32_t x, uint32_t y, Color color) {
        if (x >= width || y >= height)
            return;
        uint32_t* pixel = (uint32_t*)(base + y * pitch + x * 4);
        *pixel = get_color_hex(color);
    }

    void set_background(Color color){
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                draw_pixel(x, y, Color::WHITE);
            }
        }
    }
    
    private:

    uint8_t* get_glyph(uint8_t c) {
        return glyphs + (c * 16);
    }
};

