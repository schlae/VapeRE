#ifndef LCD_H
#define LCD_H

// Font definition table
typedef struct {
    const uint16_t count;
    const uint8_t height;
    const uint8_t *widths;
    const uint16_t *offsets;
    const uint8_t *data;
} font_def_t;

// Icon definition table
typedef struct {
    const uint8_t width;
    const uint8_t height;
    const uint16_t *pal;
    const uint8_t *image;
    const uint8_t *mask;
} ico_def_t;

void st7789_init();
void st7789_sleep();
void st7789_fill(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height, uint16_t col);
void font_string(uint16_t x, uint16_t y, char *text, uint16_t max_len,
                 uint16_t fg_color, uint16_t bg_color,
                 const font_def_t *font, bool bold);
uint16_t font_string_width(char *text, uint16_t max_len, const font_def_t *font, bool bold);
void st7789_bitblt_nor(uint16_t sx, uint16_t sy, uint32_t address);
void draw_icon(uint16_t sx, uint16_t sy, const ico_def_t *ico);
void st7789_bitblt(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height, uint16_t *buf);

#endif
