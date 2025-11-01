// Copyright (C) 2025 Tube Time. See LICENSE file for license.

// Display on STW2915 is 76 x 284, image format is BRG 5:5:6.
// Controller is probably a NewVision NV3029

#include <stdio.h>
#include <stdbool.h>
#include "py32f0xx.h"

#include "lcd.h"
#include "nor.h"

#define LCD_PWR GPIO_PIN_8
#define LCD_DC GPIO_PIN_15
#define LCD_CS GPIO_PIN_7
#define LCD_RST GPIO_PIN_6
#define LCD_BACKLIGHT GPIO_PIN_8

#define CMD_NOP         0x00
#define CMD_PRIVATE     0xFD
#define CMD_SLPIN       0x10
#define CMD_SLPOUT      0x11
#define CMD_DISPOFF     0x28
#define CMD_DISPON      0x29
#define CMD_CASET       0x2A
#define CMD_RASET       0x2B
#define CMD_RAMWR       0x2C
#define CMD_TEON        0x35
#define CMD_MADCTL      0x36
#define CMD_COLMOD      0x3A
#define CMD_PRCTR       0xB5

static SPI_HandleTypeDef hspi;
static DMA_HandleTypeDef hdma;

static uint16_t _x_offset = 0;
static uint16_t _y_offset = 0;


void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hspi.hdmatx);
}


void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi);
}


static void st7789_gpio_init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_DMA_CLK_ENABLE();
    // Turn on 3.3V to LCD, etc. 
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = LCD_PWR});
    HAL_GPIO_WritePin(GPIOA, LCD_PWR, GPIO_PIN_RESET);

    // Turn on LCD backlight
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = LCD_BACKLIGHT});
    HAL_GPIO_WritePin(GPIOB, LCD_BACKLIGHT, GPIO_PIN_RESET);
    // Configure SPI for LCD
    // DI = PB5
    // CLK = PB3
    // RST = PB6
    // CS = PB7
    // C#/D = PA15
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF0_SPI1, .Pin = GPIO_PIN_5});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF0_SPI1, .Pin = GPIO_PIN_3});

    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = LCD_RST});
    HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = LCD_CS});
    HAL_GPIO_WritePin(GPIOB, LCD_CS, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = LCD_DC});
    HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_RESET);
}


static void spi1_init()
{
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();

    hdma.Instance = DMA1_Channel1;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma.Init.Mode = DMA_NORMAL;
    hdma.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma);

    __HAL_LINKDMA(&hspi, hdmatx, hdma);

    HAL_DMA_ChannelMap(&hdma, DMA_CHANNEL_MAP_SPI1_TX);

    hspi.Instance = SPI1;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.SlaveFastMode = SPI_SLAVE_FAST_MODE_DISABLE;
    HAL_SPI_Init(&hspi);

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}


static inline void mode_cmd()
{
    HAL_GPIO_WritePin(GPIOA, LCD_DC, GPIO_PIN_RESET);
}


static inline void mode_data()
{
    HAL_GPIO_WritePin(GPIOA, LCD_DC, GPIO_PIN_SET);
}


static inline void cs_low()
{
    HAL_GPIO_WritePin(GPIOB, LCD_CS, GPIO_PIN_RESET);
}


static inline void cs_high()
{
    HAL_GPIO_WritePin(GPIOB, LCD_CS, GPIO_PIN_SET);
}


static void write_command_cs(uint8_t cmd, uint8_t num_bytes, uint8_t buf[])
{
    cs_low();
    mode_cmd();
    HAL_SPI_Transmit(&hspi, &cmd, 1, 5000);
    cs_high();
    cs_high();
    while (num_bytes-- > 0) {
        mode_data();
        cs_low();
        HAL_SPI_Transmit(&hspi, buf++, 1, 5000);
        cs_high();
    }
}


static void write_command(uint8_t cmd, uint8_t num_bytes, uint8_t buf[], bool cs)
{
    if (cs) cs_low();
    mode_cmd();
    HAL_SPI_Transmit(&hspi, &cmd, 1, 5000);
    mode_data();
    if (num_bytes > 0) {
        HAL_SPI_Transmit(&hspi, buf, num_bytes, 5000);
    }
    if (cs) cs_high();
}


// Initialize LCD panel (probably with a NewVision NV3029 controller)
void st7789_disp_init()
{
    _x_offset = 18;
    _y_offset = 82;
    HAL_GPIO_WritePin(GPIOB, LCD_CS, GPIO_PIN_SET);
    HAL_Delay(24);
    HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_SET);
    HAL_Delay(6);
    HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_SET);
    HAL_Delay(100); // Needed delay to allow LCD to init properly.

    // Configure registers
    write_command_cs(CMD_PRIVATE, 2, (uint8_t []){ 0x06, 0x08 }); // Unlock private registers

    write_command_cs(0x61, 2, (uint8_t []){ 0x07, 0x07 });
    write_command_cs(0x73, 1, (uint8_t []){ 0x70 });
    write_command_cs(0x73, 1, (uint8_t []){ 0x00 });
    write_command_cs(0x62, 3, (uint8_t []){ 0x00, 0x44, 0x40 });
    write_command_cs(0x63, 4, (uint8_t []){ 0x41, 0x02, 0x12, 0x12 });
    write_command_cs(0x64, 1, (uint8_t []){ 0x37 });
    write_command_cs(0x65, 3, (uint8_t []){ 0x09, 0x10, 0x21 });
    write_command_cs(0x66, 3, (uint8_t []){ 0x09, 0x10, 0x21 });
    write_command_cs(0x67, 2, (uint8_t []){ 0x20, 0x20 });
    write_command_cs(0x68, 4, (uint8_t []){ 0x90, 0x30, 0x20, 0x10 });

    write_command_cs(0xB1, 3, (uint8_t []){ 0x0F, 0x02, 0x01 });
    write_command_cs(0xB4, 1, (uint8_t []){ 0x01 });
    write_command_cs(CMD_PRCTR, 4, (uint8_t []){ 0x02, 0x02, 0x0A, 0x14 }); // 2 lines FP, BP. 10 pix FP, 20 pix BP
    write_command_cs(0xB6, 5, (uint8_t []){ 0x04, 0x01, 0x9F, 0x00, 0x02 });

    write_command_cs(0xE6, 2, (uint8_t []){ 0x00, 0xFF });
    write_command_cs(0xE7, 6, (uint8_t []){ 0x01, 0x04, 0x03, 0x03, 0x00, 0x12 });
    write_command_cs(0xEC, 1, (uint8_t []){ 0x52 });
    write_command_cs(0xDF, 1, (uint8_t []){ 0x11 });
    write_command_cs(0xE8, 3, (uint8_t []){ 0x00, 0x70, 0x00 });

    // Gamma parameters?
    write_command_cs(0xE2, 5, (uint8_t []){ 0x01, 0x06, 0x11, 0x1E, 0x1B });
    write_command_cs(0xE5, 6, (uint8_t []){ 0x3F, 0x1B, 0x1D, 0x11, 0x05, 0x01 });
    write_command_cs(0xE1, 2, (uint8_t []){ 0x3C, 0x6B });
    write_command_cs(0xE4, 2, (uint8_t []){ 0x6C, 0x39 });
    write_command_cs(0xE0, 8, (uint8_t []){ 0x10, 0x11, 0x11, 0x14, 0x15, 0x14, 0x10, 0x14 });
    write_command_cs(0xE3, 8, (uint8_t []){ 0x14, 0x10, 0x15, 0x16, 0x14, 0x11, 0x11, 0x10 });

    
    write_command_cs(0xF6, 4, (uint8_t []){ 0x01, 0x30, 0x00, 0x00 });
    write_command_cs(0xF1, 3, (uint8_t []){ 0x01, 0x01, 0x02 });

    write_command_cs(CMD_PRIVATE, 2, (uint8_t []){ 0xFA, 0xFC }); // Lock private registers

    // Compatible init
    write_command_cs(CMD_COLMOD, 1, (uint8_t []){ 0x55 }); // 16 BPP
    write_command_cs(CMD_TEON, 1, (uint8_t []){ 0x00 });   // Tearing effect off
    write_command_cs(CMD_MADCTL, 1, (uint8_t []){ 0x40 }); // C0 originally. Row address order, column address order
    write_command_cs(CMD_SLPOUT, 0, NULL); // Exit sleep mode

    write_command_cs(CMD_CASET, 2, (uint8_t []){ 0x00, 0x00 }); // Set up column address
    write_command_cs(CMD_RASET, 4, (uint8_t []){ 0x00, 0x11, 0x01, 0x2D }); // Set up row address
    write_command_cs(CMD_RAMWR, 0, NULL); // Dummy write
//    write_command_cs(CMD_DISPOFF, 0, NULL); Original code does this, then clears buffer, then turns it on
    HAL_Delay(6);
    write_command_cs(CMD_DISPON, 9, NULL); // Display on
}


static void st7789_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool cs)
{
    uint16_t sx = x + _x_offset;
    uint16_t sy = y + _y_offset;
    uint16_t ex = x + width - 1 + _x_offset;
    uint16_t ey = y + height - 1 + _y_offset;
    write_command(CMD_CASET, 4, (uint8_t []){sy >> 8, sy & 0xff, ey >> 8, ey & 0xff}, cs);
    write_command(CMD_RASET, 4, (uint8_t []){sx >> 8, sx & 0xff, ex >> 8, ex & 0xff}, cs);
}


// Draw a filled rectangle.
void st7789_fill(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height, uint16_t col)
{
    int count = width * height;
    int i;
    uint16_t buffer[64];

    // Fill buffer
    for (i = 0; i < 64; i++) {
        buffer[i] = col;
    }

    cs_low();
    st7789_window(sx, sy, width, height, false);
    write_command(CMD_RAMWR, 0, NULL, false);
    
    while (count > 0) {
            HAL_SPI_Transmit_DMA(&hspi, (uint8_t *)buffer, (count > 64) ? 128 : (count << 1));
            while(hspi.State != HAL_SPI_STATE_READY){}
            if (count > 64) {
                count -= 64;
            } else {
                break;
            }
    }
    cs_high();
}


// Get the width of a string using a particular font
uint16_t font_string_width(char *text, uint16_t max_len, const font_def_t *font, bool bold)
{
    char *text_buf = text;
    uint16_t total_width = 0;

    while (*text_buf) {
        if (*text_buf >= font->count) {
            text_buf++;
            continue;
        }
        if (text_buf >= text + max_len) {
            break;
        }
        total_width += font->widths[*(text_buf++)] + (bold ? 1 : 0);
    }
    return total_width;
}


// Draws a string at the specific coordinates using the default font
void font_string(uint16_t x, uint16_t y, char *text, uint16_t max_len,
                 uint16_t fg_color, uint16_t bg_color,
                 const font_def_t *font, bool bold)
{
    uint8_t row, col;
    uint16_t total_width, width;
    uint16_t offset;
    uint8_t bytes_column;
    uint8_t byte;
    uint8_t db;
    uint8_t col_count;
    char *text_buf = text;
    uint8_t prev_bit;

    // First, compute total width
    total_width = font_string_width(text, max_len, font, bold);

    for (row = 0; row < font->height; row++) {
        // Set the window
        cs_low();
        st7789_window(x, y + row, total_width, 1, false);
        write_command(CMD_RAMWR, 0, NULL, false);

        text_buf = text;
        while (*text_buf) {
            // Get information about this character
            if (*text_buf >= font->count) {
                text_buf++; // Skip if invalid
                continue;
            }
            if (text_buf >= text + max_len) {
                break;
            }
            width = font->widths[*text_buf];
            bytes_column = (width + 7) >> 3;
            offset = font->offsets[*text_buf];
            // Get exact byte offset into the character table
            offset += row * bytes_column;
            prev_bit = 0;
            for (byte = 0; byte < bytes_column; byte++) {
                db = font->data[offset + byte];
                col_count = (width > 8) ? 8 : width;
                for (col = 0; col < col_count; col++) {
                    if ((db & 0x1) || (bold && (prev_bit == 1))) {
                        // Emit foreground color
                        HAL_SPI_Transmit(&hspi, (uint8_t *)&fg_color, 2, 5000);
                    } else {
                        // Emit background color
                        HAL_SPI_Transmit(&hspi, (uint8_t *)&bg_color, 2, 5000);
                    }
                    prev_bit = db & 0x1;
                    db = db >> 1;
                }
               width -= col_count;
            }
            if (bold) {
                if (prev_bit) {
                    HAL_SPI_Transmit(&hspi, (uint8_t *)&fg_color, 2, 5000);
                } else {
                    HAL_SPI_Transmit(&hspi, (uint8_t *)&bg_color, 2, 5000);
                }
            }

            text_buf++;
        }
        cs_high();
    }
}


// Plots a bitmap. Must be 16bpp and match the display type (BGR 565)
void st7789_bitblt(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height, uint16_t *buf)
{
    cs_low();
    st7789_window(sx, sy, width, height, false);
    write_command(CMD_RAMWR, 0, NULL, false);
    HAL_SPI_Transmit_DMA(&hspi, (uint8_t *)buf, width * height * 2);
    while(hspi.State != HAL_SPI_STATE_READY){}
    cs_high();
}


#pragma pack(push, 1)
typedef struct {
    char magic[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t pixel_offset;
    uint32_t dib_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_ppm;
    uint32_t y_ppm;
    uint32_t colors;
    uint32_t imp_colors;
    uint32_t red_bm;
    uint32_t green_bm;
    uint32_t blue_bm;
    uint32_t alpha_bm;
} bmp_t;
#pragma pack(pop)


// Very basic bitblit from NOR flash
// Ignores upside-down BMP files
// 16-bit color only, but files must be byte-swapped to look correct.
void st7789_bitblt_nor(uint16_t sx, uint16_t sy, uint32_t address)
{
    bmp_t bmp_hdr;
    uint32_t count = 0;
    uint32_t img_offset = 0;
    uint8_t buffer[2048];

    // Try a bmp
    nor_read(address, (uint8_t *)&bmp_hdr, sizeof(bmp_hdr));
    // TODO: validate header
    count = bmp_hdr.image_size;
    img_offset = address + bmp_hdr.pixel_offset;

    cs_low();
    // Swap height and width since our LCD config is weird.
    st7789_window(sx, sy, bmp_hdr.height, bmp_hdr.width, false); // Swap height and width
    write_command(CMD_RAMWR, 0, NULL, false);
    
    while (count > 0) {
            nor_read(img_offset, buffer, sizeof(buffer));
            HAL_SPI_Transmit_DMA(&hspi, buffer, (count > sizeof(buffer)) ? sizeof(buffer) : count);
            while(hspi.State != HAL_SPI_STATE_READY){}
            if (count > sizeof(buffer)) {
                count -= sizeof(buffer);
                img_offset += sizeof(buffer);
            } else {
                break;
            }
    }
    cs_high();

}


void st7789_init ()
{
    uint32_t i;
    uint16_t buffer[64];

    for (i = 0; i < 64; i++) {
        buffer[i] = 0x07C0;
    }

    st7789_gpio_init();
    spi1_init();
    st7789_disp_init();
    st7789_fill(0, 0, 284, 76, 0x001F);
#if 0
    while(1) {
        //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
        //HAL_SPI_Transmit(&hspi, &test, 1, 1000);
        cs_low();
        write_command(CMD_CASET, 4, (uint8_t []){ 0x00, 0x52, 0x00, 0x9E }, false);
        write_command(CMD_RASET, 4, (uint8_t []){ 0x00, 0x12, 0x01, 0x2E }, false);
        write_command(CMD_RAMWR, 0, NULL, false);
     //   for (i = 0; i < (76 * 288); i++) {
     //       HAL_SPI_Transmit(&hspi, (uint8_t *)&test1, 2, 5000);
     //   }
        //for (i = 0; i < (76 * 284)/2; i++) {
        //    HAL_SPI_Transmit(&hspi, (uint8_t *)&test2, 2, 5000);
       //}

        // Try out DMA
        for (i = 0; i < (76 * 288) * 2 / 64; i++) {
            HAL_SPI_Transmit_DMA(&hspi, (uint8_t *)buffer, 128);
            while(hspi.State != HAL_SPI_STATE_READY){}

        }

        cs_high();
        HAL_Delay(1);
//        write_command_cs(0x29, 0, NULL);
        HAL_Delay(1000);
        // Max Y size is 76. Y offset is 82.
        // Max X size is 284. X offset is 18
        lcd_fill(0, 0, 283, 2, 0x003F);
        HAL_Delay(1000);
    }
#endif
}


void st7789_sleep()
{

    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_DisableIRQ(SPI1_IRQn);

    HAL_DMA_DeInit(&hdma);
    HAL_SPI_DeInit(&hspi);

    __HAL_RCC_SPI1_CLK_DISABLE();
    __HAL_RCC_DMA_CLK_DISABLE();
 
    // Turn off LCD backlight
    HAL_GPIO_WritePin(GPIOB, LCD_BACKLIGHT, GPIO_PIN_SET);

    // Float LCD controller pins
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_5});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_3});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = LCD_RST});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = LCD_CS});
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = LCD_DC});
}
