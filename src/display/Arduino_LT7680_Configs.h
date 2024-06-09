#ifndef _ARDUINO_LT7680_CONFIGS_H_
#define _ARDUINO_LT7680_CONFIGS_H_

#include "Arduino_LT7680.h"

// This is an example configuration for an EastRising ER-TFT3.71-1 TFT LCD
// It is a 240 x 960 pixel IPS panel with a diagonal size of 3.71"
// Details available: https://www.buydisplay.com/bar-type-3-71-inch-240x960-ips-tft-lcd-display-spi-rgb-interface


static const uint8_t er5517_init_operations[] = {
    BEGIN_WRITE,
    // WRITE_COMMAND_8, 0xff,
    // WRITE_BYTES, 5,  0x77, 0x01, 0x00, 0x00, 0x00,
    // WRITE_C8_D8, 0x01,0x00,
    // DELAY, 5,
    // WRITE_COMMAND_8, 0xff,
    // WRITE_BYTES, 5,         0x77, 0x01, 0x00, 0x00, 0x13,
    // WRITE_C8_D8,     0xef,  0x08,
    WRITE_COMMAND_8, 0xff,
    WRITE_BYTES, 5,         0x77, 0x01, 0x00, 0x00, 0x10,
    WRITE_COMMAND_8, 0xc0,                  // Display Line Setting
    WRITE_BYTES, 2,         0x77, 0x00,     // (0x77 + 1) * 8 = 960
    WRITE_COMMAND_8, 0xc1,                  // Porch Control
    WRITE_BYTES, 2,         0x11, 0x0c,     // VBP: 17, VFP: 12
    WRITE_COMMAND_8, 0xc2,                  // Inversion selection and Frame Rate Control
    WRITE_BYTES, 2,         0x37, 0x02,     // Column inversion, Min PCLK per line = 512 + (0x02 x 16) = 544

    WRITE_COMMAND_8, 0xC3,                  // RGB Control
    WRITE_BYTES, 3,         0x80, 0x10, 0x10,   // RGB HV Mode, VSYNC Pol = Active Low, HSYNC Pol = Active Low, DOTCLK Pol = Positive Edge, Enable Pol = Active High, VSYNC BP = 16, HSYNC BP = 16

    WRITE_C8_D8,     0xcc,  0x30,
    WRITE_COMMAND_8, 0xB0,
    WRITE_BYTES, 16,        0x06, 0xCF, 0x14, 0x0C, 0x0F, 0x03, 0x00, 0x0A, 0x07, 0x1B, 0x03, 0x12, 0x10, 0x25, 0x36, 0x1E,
    WRITE_COMMAND_8, 0xB1,
    WRITE_BYTES, 16,        0x0C, 0xD4, 0x18, 0x0C, 0x0E, 0x06, 0x03, 0x06, 0x08, 0x23, 0x06, 0x12, 0x10, 0x30, 0x2F, 0x1F,
    WRITE_COMMAND_8, 0xff,
    WRITE_BYTES, 5,         0x77, 0x01, 0x00, 0x00, 0x11,
    WRITE_C8_D8,     0xb0,  0x73,
    WRITE_C8_D8,     0xb1,  0x7C,
    WRITE_C8_D8,     0xb2,  0x83,
    WRITE_C8_D8,     0xb3,  0x80,
    WRITE_C8_D8,     0xb5,  0x49,
    WRITE_C8_D8,     0xb7,  0x87,
    WRITE_C8_D8,     0xb8,  0x33,
    WRITE_COMMAND_8, 0xb9,
    WRITE_BYTES, 2,         0x10, 0x1f,
    WRITE_C8_D8,     0xbb,  0x03,
    WRITE_C8_D8,     0xc1,  0x08,
    WRITE_C8_D8,     0xc2,  0x08,
    WRITE_C8_D8,     0xd0,  0x88,
    WRITE_COMMAND_8, 0xe0,
    WRITE_BYTES, 6,         0x00, 0x00, 0x02, 0x00, 0x00, 0x0c,
    WRITE_COMMAND_8, 0xe1,
    WRITE_BYTES, 11,        0x05, 0x96, 0x07, 0x96, 0x06, 0x96, 0x08, 0x96, 0x00, 0x44, 0x44,
    WRITE_COMMAND_8, 0xe2,
    WRITE_BYTES, 12,        0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,
    WRITE_COMMAND_8, 0xe3,
    WRITE_BYTES, 4,         0x00, 0x00, 0x33, 0x33,
    WRITE_COMMAND_8, 0xe4,
    WRITE_BYTES, 2,         0x44, 0x44,
    WRITE_COMMAND_8, 0xe5,
    WRITE_BYTES, 16,        0x0d, 0xd4, 0x28, 0x8c, 0x0f, 0xd6, 0x28, 0x8c, 0x09, 0xd0, 0x28, 0x8c, 0x0b, 0xd2, 0x28, 0x8c,
    WRITE_COMMAND_8, 0xe6,
    WRITE_BYTES, 4,         0x00, 0x00, 0x33, 0x33,
    WRITE_COMMAND_8, 0xe7,
    WRITE_BYTES, 2,         0x44, 0x44,
    WRITE_COMMAND_8, 0xe8,
    WRITE_BYTES, 16,        0x0e, 0xd5, 0x28, 0x8c, 0x10, 0xd7, 0x28, 0x8c, 0x0a, 0xd1, 0x28, 0x8c, 0x0c, 0xd3, 0x28, 0x8c,
    WRITE_COMMAND_8, 0xeb,
    WRITE_BYTES, 6,         0x00, 0x01, 0xe4, 0xe4, 0x44, 0x00,
    WRITE_COMMAND_8, 0xed,
    WRITE_BYTES, 16,        0xf3, 0xc1, 0xba, 0x0f, 0x66, 0x77, 0x44, 0x55, 0x55, 0x44, 0x77, 0x66, 0xf0, 0xab, 0x1c, 0x3f,
    WRITE_COMMAND_8, 0xef,
    WRITE_BYTES, 6,         0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f,
    WRITE_COMMAND_8, 0xff,
    WRITE_BYTES, 5,         0x77, 0x01, 0x00, 0x00, 0x13,
    WRITE_COMMAND_8, 0xe8,
    WRITE_BYTES, 2,         0x00, 0x0e,

    WRITE_COMMAND_8, 0xe8,
    WRITE_BYTES, 2,         0x00, 0x0c,
    DELAY, 10,
    WRITE_COMMAND_8, 0xe8,
    WRITE_BYTES, 2,         0x40, 0x00,
    WRITE_COMMAND_8, 0xff,
    WRITE_BYTES, 5,         0x77, 0x01, 0x00, 0x00, 0x00,

    WRITE_C8_D8,     0x36,  0x00,
    WRITE_C8_D8,     0x3A,  0x66,
    WRITE_COMMAND_8, 0x11,
    DELAY, 120,
    WRITE_COMMAND_8, 0x29,
    DELAY, 20,
    END_WRITE
};

LT7680_Config ER5517 {
    .PLL = {
        .CPLL = {     // 100 MHz
            .OD = 2,
            .R = 5,
            .N = 100
        },
        .MPLL = {     // 100 MHz
            .OD = 2,
            .R = 5,
            .N = 100
        },
        .PPLL = {     // 25 MHz
            .OD = 2,
            .R = 5,
            .N = 25
        }
    },
    .clock_config_sel = PLL_PARAMETERS,
    .PCLK_active_edge = RISING_EDGE,
    .DE_polarity = ACTIVE_HIGH,

    .HSYNC_polarity = ACTIVE_LOW,
    .HSYNC_front_porch = 5,
    .HSYNC_pulse_width = 5,
    .HSYNC_back_porch = 128,

    .VSYNC_polarity = ACTIVE_LOW,
    .VSYNC_front_porch = 5,
    .VSYNC_pulse_width = 5,
    .VSYNC_back_porch = 11,

    .horizontal_width = 240,
    .vertical_height = 960,
    .colour_width = WIDTH_18_BITS,
    .rotation = ROTATE_NORMAL,
    .mem_colour_depth = DEPTH_16BPP,

    .lcd_init = {
        .speed = 1000000,
        .mode = SPI_MODE0,
        .initial_operations = er5517_init_operations,
        .init_op_length = sizeof(er5517_init_operations)
    }
};

LT7680_Config ER5517_V2 {
    .FreqMHz = {
        .CCLK = 100,
        .MCLK = 100,
        .PCLK = 25,
    },
    .clock_config_sel = FREQUENCY_MHZ,
    .PCLK_active_edge = RISING_EDGE,
    .DE_polarity = ACTIVE_HIGH,

    .HSYNC_polarity = ACTIVE_LOW,
    .HSYNC_front_porch = 5,
    .HSYNC_pulse_width = 5,
    .HSYNC_back_porch = 128,

    .VSYNC_polarity = ACTIVE_LOW,
    .VSYNC_front_porch = 5,
    .VSYNC_pulse_width = 5,
    .VSYNC_back_porch = 11,

    .horizontal_width = 240,
    .vertical_height = 960,
    .colour_width = WIDTH_18_BITS,
    .rotation = ROTATE_NORMAL,
    .mem_colour_depth = DEPTH_16BPP,

    .lcd_init = {
        .speed = 1000000,
        .mode = SPI_MODE0,
        .initial_operations = er5517_init_operations,
        .init_op_length = sizeof(er5517_init_operations)
    }
};


#endif // ifndef _ARDUINO_LT7680_CONFIGS_H_