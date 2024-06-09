/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#ifndef _ARDUINO_LT7680_H_
#define _ARDUINO_LT7680_H_

#include <Arduino.h>
#include <SPI.h>
#include "Arduino_Databus.h"

#define LT7680_PROCESSING_TIME_US   5000    // Delay to allow time for the chip to process a drawing command.

#if !defined(LT7680A_R) && !defined(LT7680B_R)
// Default to LT7680A_R variant of chip
// Use "#define LT7680B_R 1" for the other type
#define LT7680A_R 1
#endif // !defined(LT7680A_R) && !defined(LT7680B_R)

//#define DEBUG_WITH_SERIAL

#ifdef DEBUG_WITH_SERIAL
#define SERIAL_PRINTF(args...) Serial.printf(args);
#else
#define SERIAL_PRINTF(args...)  
#endif

enum LT7680_PCLKActiveEdge : uint8_t {
    RISING_EDGE = 0,
    FALLING_EDGE = 1
};

enum LT7680_Polarity : uint8_t {
    ACTIVE_LOW = 0,
    ACTIVE_HIGH = 1
};


enum LT7680_TFTColorDataWidth : uint8_t {
    WIDTH_24_BITS = 0,
    WIDTH_18_BITS = 1,
    WIDTH_16_BITS = 2,
    NO_TFT_OUTPUT = 3
};

enum LT7680_ColourDepth : uint8_t {
    DEPTH_8BPP  = 0,    // 8 bpp
    DEPTH_16BPP = 1,    // 16 bpp
    DEPTH_24BPP = 2,     // 24 bpp
};

enum LT7680_Rotation : uint8_t {
    ROTATE_NORMAL = 0,
    ROTATE_RIGHT_90,
    ROTATE_180,
    ROTATE_LEFT_90
};

enum LT7680_PLL_Config_Selection : uint8_t {
    PLL_PARAMETERS = 0,
    FREQUENCY_MHZ = 1
};

enum LT7680_CharacterGeneratorSource : uint8_t {
    CGROM_INTERNAL = 0,
    CGROM_EXTERNAL = 1,
    CGROM_USER_DEFINED = 2
};

enum LT7680_CharacterHeight : uint8_t {
    CHAR_HEIGHT_DOTS_16 = 0,
    CHAR_WEIGHT_DOTS_24 = 1,
    CHAR_HEIGHT_DOTS_32 = 2,
};

enum LT7680_CharacterSelection : uint8_t {
    CHAR_ISO_IEC_8859_1 = 0,
    CHAR_ISO_IEC_8859_2 = 1,
    CHAR_ISO_IEC_8859_4 = 2,
    CHAR_ISO_IEC_8859_5 = 3,
};

// Configuration of the direct LCD SPI connection (not the one to the LT7680)
struct LCD_SPI_Config {
    int32_t     speed;  // Speed of the SPI clock
    int8_t      mode;   // Mode of the SPI datalink
    const uint8_t *   initial_operations; // Pointer to the initial commands to send to LCD
    uint16_t    init_op_length; // Length of the initial commands set
};

struct PLL_Config {
    uint8_t     OD; // Output Divider Ratio
    uint8_t     R;  // Input Divider Ratio
    uint16_t    N;  // Feedback Divider Ratio
};

struct Freq_Config {
    uint16_t CCLK;  // Required frequency in MHz
    uint16_t MCLK;  // Required frequency in MHz
    uint16_t PCLK;  // Required frequency in MHz
};
// Fout = XI * (N / R) / OD, where XI = 10 MHz
// Rule of thumb is:
// 1. CCLK * 2 >= MCLK >= PCLK
// 2. CCLK >= PCLK * 1.5

struct LT7680_Config {

    union
    {
        struct {
            PLL_Config CPLL;   // Provides CCLK for host interface, BTE Engine, Graphics Engine, Text DMA transfer, etc.
            PLL_Config MPLL;   // Provides MCLK for internal Display RAM
            PLL_Config PPLL;   // Provides PCLK for TFT-LCD Scan Clock
        } PLL;

        Freq_Config FreqMHz;
    };

    LT7680_PLL_Config_Selection clock_config_sel;   // Determines if the clock config is given in PLL parameters, or just the frequencies
    LT7680_PCLKActiveEdge   PCLK_active_edge;       // Active edge of PCLK
    LT7680_Polarity         DE_polarity;            // Whether DE is active high or low
    LT7680_Polarity         HSYNC_polarity;         // Whether HSYNC is active high or low
    uint8_t                 HSYNC_front_porch; 
    uint8_t                 HSYNC_pulse_width;
    uint8_t                 HSYNC_back_porch;
    LT7680_Polarity         VSYNC_polarity;         // Whether VSYNC is active high or low
    uint8_t                 VSYNC_front_porch; 
    uint8_t                 VSYNC_pulse_width;
    uint8_t                 VSYNC_back_porch;

    uint16_t                horizontal_width;       // Number of Horizontal Pixels
    uint16_t                vertical_height;        // Number of Vertical Pixels
    LT7680_TFTColorDataWidth colour_width;          // Sets the colour data bus width
    LT7680_Rotation         rotation;               // Sets the image rotation in hardware
    LT7680_ColourDepth      mem_colour_depth;       // Sets the colour depth of the LT7680 display memory
    LCD_SPI_Config          lcd_init;               // Inforamtion on initialising the LCD
};

#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#ifdef LT7680A_R
#define LT7680_TFTWIDTH 1280  ///< LT7680A-R max TFT width
#define LT7680_TFTHEIGHT 1024 ///< LT7680A-R max TFT height
#endif // ifdef LT7680A_R
#ifdef LT7680B_R
#define LT7680_TFTWIDTH 480  ///< LT7680B-R max TFT width
#define LT7680_TFTHEIGHT 320 ///< LT7680B-R max TFT height
#endif // ifdef LT7680A_R

#define LT7680_SDRAR_SETTING        0x29 // Four banks, Row size 4096, Column Size 512
#define LT7680_SDR_REF_LO_SETTING   0x1A // SDRAM Auto Refresh Timing for chip (LSB)
#define LT7680_SDR_REF_HI_SETTING   0x06 // SDRAM Auto Refresh Timing for chip (MSB)

#define LT7680_RST_DELAY            100    // In ms

// Configuration Registers
#define LT7680_SRR                  0x00 // Software Reset Register (SRR)
#define LT7680_CCR                  0x01 // Chip Configuration Register (CCR)
#define LT7680_MACR                 0x02 // Memory Access Control Register (MACR)
#define LT7680_ICR                  0x03 // Input Control Register (ICR)
#define LT7680_MRWDP                0x04 // Memory Data Read/Write Port (MRWDP)

// PLL Setting Register
#define LT7680_PPLLC1               0x05 // PCLK PLL Control Register 1 (PPLLC1)
#define LT7680_PPLLC2               0x06 // PCLK PLL Control Register 2 (PPLLC2)
#define LT7680_MPLLC1               0x07 // MCLK PLL Control Register 1 (MPLLC1)
#define LT7680_MPLLC2               0x08 // MCLK PLL Control Register 2 (MPLLC2)
#define LT7680_CPLLC1               0x09 // CCLK PLL Control Register 1 (CPLLC1)
#define LT7680_CPLLC2               0x0A // CCLK PLL Control Register 2 (CPLLC2)

// Interrupt Control Registers
#define LT7680_INTEN                0x0B // Interrupt Enable Register (INTEN)
#define LT7680_INTF                 0x0C //  Interrupt Event Flag Register (INTF)
#define LT7680_MINTFR               0x0D // Mask Interrupt Flag Register (MINTFR)
#define LT7680_PUENR                0x0E //  Pull-High Control Register (PUENR)
#define LT7680_PSFSR                0x0F // PD for GPIO/Key Function Select Register (PSFSR)

// LCD Display Control Registers
#define LT7680_MPWCTR               0x10 // Main/PIP Window Control Register (MPWCTR)
#define LT7680_PIPCDEP              0x11 // PIP Window Color Depth Setting (PIPCDEP)
#define LT7680_DPCR                 0x12 // Display Configuration Register (DPCR)
#define LT7680_PCSR                 0x13 // Panel Scan Clock and Data Setting Register (PCSR)
#define LT7680_HDWR                 0x14 // Horizontal Display Width Register (HDWR)
#define LT7680_HDWFTR               0x15 // Horizontal Display Width Fine Tune Register (HDWFTR)
#define LT7680_HNDR                 0x16 // Horizontal Non-Display Period Register (HNDR) a.k.a "back porch"
#define LT7680_HNDFTR               0x17 // Horizontal Non-Display Period Fine Tune Register (HNDFTR)
#define LT7680_HSTR                 0x18 // HSYNC Start Position Register (HSTR) a.k.a "front porch"
#define LT7680_HPWR                 0x19 // HSYNC Pulse Width Register (HPWR)
#define LT7680_VDHR_LO              0x1A // Vertical Display Height Register (VDHR) LSB
#define LT7680_VDHR_HI              0x1B // Vertical Display Height Register (VDHR) MSB
#define LT7680_VNDR_LO              0x1C // Vertical Non-Display Period Register (VNDR) LSB
#define LT7680_VNDR_HI              0x1D // Vertical Non-Display Period Register (VNDR) MSB
#define LT7680_VSTR                 0x1E // VSYNC Start Position Register (VSTR)
#define LT7680_VPWR                 0x1F // VSYNC Pulse Width Register (VPWR)

#define LT7680_MISA_LO              0x20 // Main Image Start Address (MISA) LSB
#define LT7680_MISA_MIDLO           0x21 // Main Image Start Address (MISA) Mid-LSB
#define LT7680_MISA_MIDHI           0x22 // Main Image Start Address (MISA) Mid-MSB
#define LT7680_MISA_HI              0x23 // Main Image Start Address (MISA) MSB
#define LT7680_MIW_LO               0x24 // Main Image Width (MIW) LSB
#define LT7680_MIW_HI               0x25 // Main Image Width (MIW) MSB
#define LT7680_MWULX_LO             0x26 // Main Window Upper-Left Corner X-Coordinates (MWULX) LSB
#define LT7680_MWULX_HI             0x27 // Main Window Upper-Left Corner X-Coordinates (MWULX) MSB
#define LT7680_MWULY_LO             0x28 // Main Window Upper-Left Corner Y-Coordinates (MWULY) LSB
#define LT7680_MWULY_HI             0x29 // Main Window Upper-Left Corner Y-Coordinates (MWULY) MSB
#define LT7680_PWDULX_LO            0x2A // PIP Window 1 or 2 Display Upper-Left Corner X-Coordinates (PWDULX) LSB
#define LT7680_PWDULX_HI            0x2B // PIP Window 1 or 2 Display Upper-Left Corner X-Coordinates (PWDULX) MSB
#define LT7680_PWDULY_LO            0x2C // PIP Window 1 or 2 Display Upper-Left Corner Y-Coordinates (PWDULY) LSB
#define LT7680_PWDULY_HI            0x2D // PIP Window 1 or 2 Display Upper-Left Corner Y-Coordinates (PWDULY) MSB
#define LT7680_PISA_LO              0x2E // PIP Image 1 or 2 Start Address (PISA) LSB
#define LT7680_PISA_MIDLO           0x2F // PIP Image 1 or 2 Start Address (PISA) Mid-LSB
#define LT7680_PISA_MIDHI           0x30 // PIP Image 1 or 2 Start Address (PISA) Mid-MSB
#define LT7680_PISA_HI              0x31 // PIP Image 1 or 2 Start Address (PISA) MSB
#define LT7680_PIW_LO               0x32 // PIP Image 1 or 2 Width (PIW) LSB
#define LT7680_PIW_HI               0x33 // PIP Image 1 or 2 Width (PIW) MSB
#define LT7680_PWIULX_LO            0x34 // PIP Window Image 1 or 2 Upper-Left Corner X-Coordinates (PWIULX) LSB
#define LT7680_PWIULX_HI            0x35 // PIP Window Image 1 or 2 Upper-Left Corner X-Coordinates (PWIULX) MSB
#define LT7680_PWIULY_LO            0x36 // PIP Window Image 1 or 2 Upper-Left Corner X-Coordinates (PWIULY) LSB
#define LT7680_PWIULY_HI            0x37 // PIP Window Image 1 or 2 Upper-Left Corner X-Coordinates (PWIULY) MSB
#define LT7680_PWW_LO               0x38 // PIP Window 1 or 2 Width (PWW) LSB
#define LT7680_PWW_HI               0x39 // PIP Window 1 or 2 Width (PWW) MSB
#define LT7680_PWH_LO               0x3A // PIP Window 1 or 2 Height (PWH) LSB
#define LT7680_PWH_HI               0x3B // PIP Window 1 or 2 Height (PWH) MSB
#define LT7680_GTCCR                0x3C // Graphic / Text Cursor Control Register (GTCCR)
#define LT7680_BTCR                 0x3D // Blink Time Control Register (BTCR)
#define LT7680_CURHS                0x3E // Text Cursor Horizontal Size Register (CURHS)
#define LT7680_CURVS                0x3F // Text Cursor Vertical Size Register (CURVS)
#define LT7680_GCHP_LO              0x40 // Graphic Cursor Horizontal Position Register (GCHP) LSB
#define LT7680_GCHP_HI              0x41 // Graphic Cursor Horizontal Position Register (GCHP) MSB
#define LT7680_GCVP_LO              0x42 // Graphic Cursor Vertical Position Register (GCVP) LSB
#define LT7680_GCVP_HI              0x43 // Graphic Cursor Vertical Position Register (GCVP) MSB
#define LT7680_GCC0                 0x44 // Graphic Cursor Color 0 (GCC0)
#define LT7680_GCC1                 0x45 // Graphic Cursor Color 1 (GCC1)

// Geometric Engine Control Registers
#define LT7680_CVSSA_LO             0x50 // Canvas Start Address (CVSSA) LSB
#define LT7680_CVSSA_MIDLO          0x51 // Canvas Start Address (CVSSA) Mid-LSB
#define LT7680_CVSSA_MIDHI          0x52 // Canvas Start Address (CVSSA) Mid-MSB
#define LT7680_CVSSA_HI             0x53 // Canvas Start Address (CVSSA) MSB
#define LT7680_CVS_IMWTH_LO         0x54 // Canvas Image Width (CVS_IMWTH) LSB
#define LT7680_CVS_IMWTH_HI         0x55 // Canvas Image Width (CVS_IMWTH) MSB
#define LT7680_AWUL_X_LO            0x56 // Active Window Upper-Left Corner X-Coordinates (AWUL_X) LSB
#define LT7680_AWUL_X_HI            0x57 // Active Window Upper-Left Corner X-Coordinates (AWUL_X) MSB
#define LT7680_AWUL_Y_LO            0x58 // Active Window Upper-Left Corner Y-Coordinates (AWUL_Y) LSB
#define LT7680_AWUL_Y_HI            0x59 // Active Window Upper-Left Corner Y-Coordinates (AWUL_Y) MSB
#define LT7680_AW_WTH_LO            0x5A // Active Window Width (AW_WTH) LSB
#define LT7680_AW_WTH_HI            0x5B // Active Window Width (AW_WTH) MSB
#define LT7680_AW_HT_LO             0x5C // Active Window Height (AW_HT) LSB
#define LT7680_AW_HT_HI             0x5D // Active Window Height (AW_HT) MSB
#define LT7680_AW_COLOR             0x5E // Color Depth of Canvas & Active Window (AW_COLOR)
#define LT7680_CURH_LO              0x5F // Graphic Read/Write X-Coordinate Register (CURH) LSB
#define LT7680_CURH_HI              0x60 // Graphic Read/Write X-Coordinate Register (CURH) MSB
#define LT7680_CURV_LO              0x61 // Graphic Read/Write Y-Coordinate Register (CURV) LSB
#define LT7680_CURV_HI              0x62 // Graphic Read/Write Y-Coordinate Register (CURV) MSB
#define LT7680_F_CURX_LO            0x63 // Text Write X-Coordinates Register (F_CURX) LSB
#define LT7680_F_CURX_HI            0x64 // Text Write X-Coordinates Register (F_CURX) MSB
#define LT7680_F_CURY_LO            0x65 // Text Write Y-Coordinates Register (F_CURY) LSB
#define LT7680_F_CURY_HI            0x66 // Text Write Y-Coordinates Register (F_CURY) MSB
#define LT7680_DCR0                 0x67 // Draw Line/Triangle Control Register 0 (DCR0)
#define LT7680_DLHSR_LO             0x68 // Draw Line/Rectangle/Triangle Point 1 X-Coordinates Register (DLHSR) LSB
#define LT7680_DLHSR_HI             0x69 // Draw Line/Rectangle/Triangle Point 1 X-Coordinates Register (DLHSR) MSB
#define LT7680_DLVSR_LO             0x6A // Draw Line/Rectangle/Triangle Point 1 Y-Coordinates Register (DLVSR) LSB
#define LT7680_DLVSR_HI             0x6B // Draw Line/Rectangle/Triangle Point 1 Y-Coordinates Register (DLVSR) MSB
#define LT7680_DLHER_LO             0x6C // Draw Line/Rectangle/Triangle Point 2 X-Coordinates Register (DLHER) LSB
#define LT7680_DLHER_HI             0x6D // Draw Line/Rectangle/Triangle Point 2 X-Coordinates Register (DLHER) MSB
#define LT7680_DLVER_LO             0x6E // Draw Line/Rectangle/Triangle Point 2 Y-Coordinates Register (DLVER) LSB
#define LT7680_DLVER_HI             0x6F // Draw Line/Rectangle/Triangle Point 2 Y-Coordinates Register (DLVER) MSB
#define LT7680_DTPH_LO              0x70 // Draw Triangle Point 3 X-Coordinates Register (DTPH) LSB
#define LT7680_DTPH_HI              0x71 // Draw Triangle Point 3 X-Coordinates Register (DTPH) MSB
#define LT7680_DTPV_LO              0x72 // Draw Triangle Point 3 X-Coordinates Register (DTPV) LSB
#define LT7680_DTPV_HI              0x73 // Draw Triangle Point 3 X-Coordinates Register (DTPV) MSB
#define LT7680_DCR1                 0x76 // Draw Circle/Ellipse/Ellipse Curve/Circle Square Control Register 1 (DCR1)
#define LT7680_ELL_A_LO             0x77 // Draw Circle/Ellipse/Rounded-Rectangle Major-Radius Register (ELL_A) LSB
#define LT7680_ELL_A_HI             0x78 // Draw Circle/Ellipse/Rounded-Rectangle Major-Radius Register (ELL_A) MSB
#define LT7680_ELL_B_LO             0x79 // Draw Circle/Ellipse/Rounded-Rectangle Minor-Radius Register (ELL_B) LSB
#define LT7680_ELL_B_HI             0x7A // Draw Circle/Ellipse/Rounded-Rectangle Minor-Radius Register (ELL_B) MSB
#define LT7680_DEHR_LO              0x7B // Draw Circle/Ellipse/Rounded-Rectangle Center X-Coordinates Register (DEHR) LSB
#define LT7680_DEHR_HI              0x7C // Draw Circle/Ellipse/Rounded-Rectangle Center X-Coordinates Register (DEHR) MSB
#define LT7680_DEVR_LO              0x7D // Draw Circle/Ellipse/Rounded-Rectangle Center Y-Coordinates Register (DEVR) LSB
#define LT7680_DEVR_HI              0x7E // Draw Circle/Ellipse/Rounded-Rectangle Center Y-Coordinates Register (DEVR) MSB
#define LT7680_FGCR                 0xD2 // Foreground Color Register - Red (FGCR)
#define LT7680_FGCG                 0xD3 // Foreground Color Register - Green (FGCG)
#define LT7680_FGCB                 0xD4 // Foreground Color Register - Blue (FGCB)

// PWM Control Registers
#define LT7680_PSCLR                0x84 // PWM Prescaler Register (PSCLR)
#define LT7680_PMUXR                0x85 // PWM Clock Mux Register (PMUXR)
#define LT7680_PCFGR                0x86 // PWM Configuration Register (PCFGR)
#define LT7680_DZ_LENGTH            0x87 // Timer-0 Dead Zone Length Register (DZ_LENGTH)
#define LT7680_TCMPB0_LO            0x88 // Timer-0 Compare Buffer Register [TCMPB0] LSB
#define LT7680_TCMPB0_HI            0x89 // Timer-0 Compare Buffer Register [TCMPB0] MSB
#define LT7680_TCNTB0_LO            0x8A // Timer-0 Count Buffer Register [TCNTB0] LSB
#define LT7680_TCNTB0_HI            0x8B // Timer-0 Count Buffer Register [TCNTB0] MSB
#define LT7680_TCMPB1_LO            0x8C // Timer-1 Compare Buffer Register [TCMPB1] LSB
#define LT7680_TCMPB1_HI            0x8D // Timer-1 Compare Buffer Register [TCMPB1] MSB
#define LT7680_TCNTB1_LO            0x8E // Timer-1 Count Buffer Register [TCNTB1] LSB
#define LT7680_TCNTB1_HI            0x8F // Timer-1 Count Buffer Register [TCNTB1] MSB

// Bit Block Transfer Engine (BTE) Control Registers
#define LT7680_BLT_CTRL0            0x90 // BTE Function Control Register 0 (BLT_CTRL0)
#define LT7680_BLT_CTRL1            0x91 // BTE Function Control Register1 (BLT_CTRL1)
#define LT7680_BLT_COLR             0x92 // Source 0/1 & Destination Color Depth (BLT_COLR)
#define LT7680_S0_STR_LO            0x93 // Source 0 Memory Start Address (S0_STR) LSB
#define LT7680_S0_STR_MIDLO         0x94 // Source 0 Memory Start Address (S0_STR) Mid-LSB
#define LT7680_S0_STR_MIDHI         0x95 // Source 0 Memory Start Address (S0_STR) Mid-MSB
#define LT7680_S0_STR_HI            0x96 // Source 0 Memory Start Address (S0_STR) MSB
#define LT7680_S0_WTH_LO            0x97 // Source 0 Image Width (S0_WTH) LSB
#define LT7680_S0_WTH_HI            0x98 // Source 0 Image Width (S0_WTH) MSB
#define LT7680_S0_X_LO              0x99 // Source 0 Window Upper-Left Corner X-Coordinates (S0_X) LSB
#define LT7680_S0_X_HI              0x9A // Source 0 Window Upper-Left Corner X-Coordinates (S0_X) MSB
#define LT7680_S0_Y_LO              0x9B // Source 0 Window Upper-Left Corner Y-Coordinates (S0_Y) LSB
#define LT7680_S0_Y_HI              0x9C // Source 0 Window Upper-Left Corner Y-Coordinates (S0_Y) MSB
#define LT7680_S1_STR_LO            0x9D // Source 1 Memory Start Address (S1_STR) LSB
#define LT7680_S1_STR_MIDLO         0x9E // Source 1 Memory Start Address (S1_STR) Mid-LSB
#define LT7680_S1_STR_MIDHI         0x9F // Source 1 Memory Start Address (S1_STR) Mid-MSB
#define LT7680_S1_STR_HI            0xA0 // Source 1 Memory Start Address (S1_STR) MSB
#define LT7680_S1_WTH_LO            0xA1 // Source 1 Image Width (S1_WTH) LSB
#define LT7680_S1_WTH_HI            0xA2 // Source 1 Image Width (S1_WTH) MSB
#define LT7680_S1_X_LO              0xA3 // Source 1 Window Upper-Left Corner X-Coordinates (S1_X) LSB
#define LT7680_S1_X_HI              0xA4 // Source 1 Window Upper-Left Corner X-Coordinates (S1_X) MSB
#define LT7680_S1_Y_LO              0xA5 // Source 1 Window Upper-Left Corner Y-Coordinates (S1_Y) LSB
#define LT7680_S1_Y_HI              0xA6 // Source 1 Window Upper-Left Corner Y-Coordinates (S1_Y) MSB
#define LT7680_DT_STR_LO            0xA7 // Destination Memory Start Address (DT_STR) LSB
#define LT7680_DT_STR_MIDLO         0xA8 // Destination Memory Start Address (DT_STR) Mid-LSB
#define LT7680_DT_STR_MIDHI         0xA9 // Destination Memory Start Address (DT_STR) Mid-MSB
#define LT7680_DT_STR_HI            0xAA // Destination Memory Start Address (DT_STR) MSB
#define LT7680_DT_WTH_LO            0xAB // Destination Image Width (DT_WTH) LSB
#define LT7680_DT_WTH_HI            0xAC // Destination Image Width (DT_WTH) MSB
#define LT7680_DT_X_LO              0xAD // Destination Window Upper-Left Corner X-Coordinates (DT_X) LSB
#define LT7680_DT_X_HI              0xAE // Destination Window Upper-Left Corner X-Coordinates (DT_X) MSB
#define LT7680_DT_Y_LO              0xAF // Destination Window Upper-Left Corner Y-Coordinates (DT_Y) LSB
#define LT7680_DT_Y_HI              0xB0 // Destination Window Upper-Left Corner Y-Coordinates (DT_Y) MSB
#define LT7680_BLT_WTH_LO           0xB1 // BTE Window Width (BLT_WTH) LSB
#define LT7680_BLT_WTH_HI           0xB2 // BTE Window Width (BLT_WTH) MSB
#define LT7680_BLT_HIG_LO           0xB3 // BTE Window Height (BLT_HIG) LSB
#define LT7680_BLT_HIG_HI           0xB4 // BTE Window Height (BLT_HIG) MSB
#define LT7680_APB_CTRL             0xB5 // Alpha Blending (APB_CTRL)

// Serial Flash and SPI Master Control Registers
#define LT7680_DMA_CTRL             0xB6 // Serial Flash DMA Controller REG (DMA_CTRL)
#define LT7680_SFL_CTRL             0xB7 // Serial Flash/ROM Controller Register (SFL_CTRL)
#define LT7680_SPIDR                0xB8 // SPI Master Tx /Rx FIFO Data Register (SPIDR)
#define LT7680_SPIMCR2              0xB9 // SPI Master Control Register (SPIMCR2)
#define LT7680_SPIMSR               0xBA // SPI Master Status Register (SPIMSR)
#define LT7680_SPI_DIVSOR           0xBB // SPI Clock period (SPI_DIVSOR)
#define LT7680_DMA_SSTR_LO          0xBC // Serial Flash DMA Source Starting Address (DMA_SSTR) LSB
#define LT7680_DMA_SSTR_MIDLO       0xBD // Serial Flash DMA Source Starting Address (DMA_SSTR) Mid-LSB
#define LT7680_DMA_SSTR_MIDHI       0xBE // Serial Flash DMA Source Starting Address (DMA_SSTR) Mid-MSB
#define LT7680_DMA_SSTR_HI          0xBF // Serial Flash DMA Source Starting Address (DMA_SSTR) MSB
#define LT7680_DMA_DX_LO            0xC0 // DMA Destination Window Upper-Left Corner X-Coordinates (DMA_DX) LSB
#define LT7680_DMA_DX_HI            0xC1 // DMA Destination Window Upper-Left Corner X-Coordinates (DMA_DX) MSB
#define LT7680_DMA_DY_LO            0xC2 // DMA Destination Window Upper-Left Corner Y-Coordinates (DMA_DY) LSB
#define LT7680_DMA_DY_HI            0xC3 // DMA Destination Window Upper-Left Corner Y-Coordinates (DMA_DY) MSB
#define LT7680_DMAW_WTH_LO          0xC6 // DMA Block Width (DMAW_WTH) LSB
#define LT7680_DMAW_WTH_HI          0xC7 // DMA Block Width (DMAW_WTH) MSB
#define LT7680_DMAW_HIGH_LO         0xC8 // DMA Block Height (DMAW_HIGH) LSB
#define LT7680_DMAW_HIGH_HI         0xC9 // DMA Block Height (DMAW_HIGH) MSB
#define LT7680_DMAW_SWTH_LO         0xCA // DMA Source Picture Width (DMA_SWTH) LSB
#define LT7680_DMAW_SWTH_HI         0xCB // DMA Source Picture Width (DMA_SWTH) MSB

// Text Engine Registers
#define LT7680_CCR0                 0xCC // Character Control Register 0 (CCR0)
#define LT7680_CCR1                 0xCD // Character Control Register 1 (CCR1)
#define LT7680_FLDR                 0xD0 // Character Line gap Setting Register (FLDR)
#define LT7680_F2FSSR               0xD1 // Character to Character Space Setting Register (F2FSSR)
#define LT7680_BGCR                 0xD5 // Background Color Register - Red (BGCR)
#define LT7680_BGCG                 0xD6 // Background Color Register - Green (BGCG)
#define LT7680_BGCB                 0xD7 // Background Color Register - Blue (BGCB)
#define LT7680_CGRAM_STR0_LO        0xDB // CGRAM Start Address 0 (CGRAM_STR0) LSB
#define LT7680_CGRAM_STR0_MIDLO     0xDC // CGRAM Start Address 0 (CGRAM_STR0) Mid-LSB
#define LT7680_CGRAM_STR0_MIDHI     0xDD // CGRAM Start Address 0 (CGRAM_STR0) Mid-MSB
#define LT7680_CGRAM_STR0_HI        0xDE // CGRAM Start Address 0 (CGRAM_STR0) MSB

// Power Management Control Register
#define LT7680_PMU                  0xDF // Power Management Register (PMU)

// Display RAM Control Register
#define LT7680_SDRAR                0xE0 // SDRAM Attribute Register (SDRAR)
#define LT7680_SDRMD                0xE1 // SDRAM Mode Register & Extended Mode Register (SDRMD)
#define LT7680_SDR_REF_LO           0xE2 // SDRAM Auto Refresh Interval (SDR_REF) LSB
#define LT7680_SDR_REF_HI           0xE3 // SDRAM Auto Refresh Interval (SDR_REF) MSB
#define LT7680_SDRCR                0xE4 // SDRAM Control Register (SDRCR)
#define LT7680_SDRAM_TIM_1          0xE0 // SDRAM Timing Parameter 1 (when bit 2 of LT7680_SDRCR is set)
#define LT7680_SDRAM_TIM_2          0xE1 // SDRAM Timing Parameter 2 (when bit 2 of LT7680_SDRCR is set)
#define LT7680_SDRAM_TIM_3          0xE2 // SDRAM Timing Parameter 3 (when bit 2 of LT7680_SDRCR is set)
#define LT7680_SDRAM_TIM_4          0xE3 // SDRAM Timing Parameter 4 (when bit 2 of LT7680_SDRCR is set)

// I2C Master Register
#define LT7680_I2CMCK_LO            0xE5 // I2C Master Clock Prescaler Register (I2CMCK) LSB
#define LT7680_I2CMCK_HI            0xE6 // I2C Master Clock Prescaler Register (I2CMCK) MSB
#define LT7680_I2CMTXR              0xE7 // I2C Master Transmit Register (I2CMTXR)
#define LT7680_I2CMRXR              0xE8 // I2C Master Receiver Register (I2CMRXR)
#define LT7680_I2CMCMD              0xE9 // I2C Master Command Register (I2CMCMD)
#define LT7680_I2CMST               0xEA // I2C Master Status Register (I2CMST)

// GPIO Register
#define LT7680_GPIOAD               0xF0 // GPIO-A Direction (GPIOAD)
#define LT7680_GPIOA                0xF1 // GPIO-A (GPIOA)
#define LT7680_GPIOB                0xF2 // GPIO-B (GPIOB)
#define LT7680_GPIOCD               0xF3 // GPIO-C Direction (GPIOCD)
#define LT7680_GPIOC                0xF4 // GPIO-C (GPIOC)
#define LT7680_GPIODD               0xF5 // GPIO-D Direction (GPIODD)
#define LT7680_GPIOD                0xF6 // GPIO-D (GPIOD)



class Arduino_LT7680 : public Arduino_TFT
{
public:
    Arduino_LT7680(Arduino_DataBus *controller, LT7680_Config config, int8_t rst = GFX_NOT_DEFINED, Arduino_DataBus *lcd_direct = nullptr);

    // Base object overrides
    bool begin(int32_t speed = GFX_NOT_DEFINED) override;

    void writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h) override;
    void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override;
    void writeSlashLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void writeColor(uint16_t color) override;
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;

    void setRotation(uint8_t r) override;
    void invertDisplay(bool) override;
    void displayOn() override;
    void displayOff() override;

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;
    void drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color) override;
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
    void fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color) override;
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillScreen(uint16_t color) override;

    void draw16bitRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) override;
    void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
    // void draw16bitRGBBitmapWithMask(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override;
    // void draw16bitRGBBitmapWithMask(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override;
    // void draw16bitRGBBitmapWithTranColor(int16_t x, int16_t y, uint16_t *bitmap, uint16_t transparent_color, int16_t w, int16_t h) override;
    // void draw24bitRGBBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override;
    // void draw24bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override;
    // void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) override;
    // void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
    // void drawIndexedBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t *color_index, int16_t w, int16_t h, int16_t x_skip = 0) override;
    // void drawIndexedBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t *color_index, uint8_t chroma_key, int16_t w, int16_t h, int16_t x_skip = 0) override;
    // void draw3bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
    // void draw16bitBeRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
    // void draw24bitRGBBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) override;
    // void draw24bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) override;
    void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) override;
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override;
    void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) override;
    void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override;

protected:
    // Base object overrides
    // From Arduino_TFT
    void tftInit() override;

public:

    enum LT7680_WindowSelection : uint8_t {
        MAIN = 0,           // Main Window (what's displayed on the screen)
        PIP1 = 1,           // Picture-in-Picture 1 Window (displayed on screen if configured)
        PIP2 = 2,           // Picture-in-Picture 2 Window (displayed on screen if configured)
        CANVAS = 3,         // Canvas Window (where the host writes to)
        BTE_S0 = 4,         // BTE Source 0
        BTE_S1 = 5,         // BTE Source 1
        BTE_DEST = 6,       // BTE Destination
        USER_CGRAM = 7,     // User-defined Character Generator RAM
        USER_SPACE_1 = 8,   // User space 1
        USER_SPACE_2 = 9,   // User space 2
        USER_SPACE_3 = 10,  // User space 3
        USER_SPACE_4 = 11,  // User space 4
    };


    enum LT7680_ColourDepth_S1 : uint8_t {
        DEPTH_8BPP  = 0,    // 8 bpp
        DEPTH_16BPP = 1,    // 16 bpp
        DEPTH_24BPP = 2,     // 24 bpp
        BTE_CONST_COLOUR = 3,    // BTE S1 Only - Constant colour
        BTE_8BIT_ALPHA_BLEND = 4, // BTE S1 Only - 8-bit pixel alpha blending
        BTE_16BIT_ALPHA_BLEND = 5 // BTE S1 Only - 16-bit pixel alpha blending
    };


    enum LT7680_Power_Mode : uint8_t {
        NORMAL=0,     // All clocks running
        STANDBY=1,    // CLCK and PLCK will stop. Memory clock (MCLK) will continue
        SUSPEND=2,    // CCLK, MCLK and PCLK will stop, but memory clock provided by oscillator clock
        SLEEP=3       // All clocks and PLL will stop operating
    };

    enum LT7680_DataWriteOperation : uint8_t {
        DISPLAY_RAM = 0,        // Image buffer (Display RAM) for image data, pattern, user-characters.
                                // Support Read-modify-Write.
        GAMMA_TABLE = 1,        // Gamma table for Color Red/Green/Blue.
                                // Each color's gamma table has 256 bytes. User need specify desired gamma table and continuous write 256 bytes.
        GRAPHIC_CURSOR_RAM = 2, // Graphic Cursor RAM (only accept low 8-bits MPU data, similar normal register data r/w.), not support Graphic Cursor RAM read.
                                // It contains 4 graphic cursor sets. Each set has 128x16 bits. User need specify target graphic cursor set and continue write 256 bytes.
        COLOR_PALETTE_RAM = 3   // Color palette RAM. It is 64x12 bits SRAM, so even address' data only low 4 bits are valid.
                                // Not support Color palette RAM read. User need continue write 128 bytes.
    };

    enum LT7680_Draw_Shape_Option : uint8_t {
        DRAW_LINE = 0,   
        DRAW_TRIANGLE = 1,
        DRAW_RECT = 2,
        DRAW_QUAD = 3,
        DRAW_PENT = 4,
        DRAW_POLY_3EP = 5,
        DRAW_POLY_4EP = 6,
        DRAW_POLY_5EP = 7,
        DRAW_ELLIPSE = 8,
        DRAW_RND_RECT = 9,
        DRAW_ARC_UPRT = 12,
        DRAW_ARC_UPLT = 13,
        DRAW_ARC_LOLT = 14,
        DRAW_ARC_LORT = 15,
    };

    enum LT7680_Status_Register_Mask : uint8_t {
        INTERRUPT_PIN_STATE = 0x01, // 0: without interrupt active, 1: interrupt active
        OPERATION_MODE      = 0x02, // 0: Normal operation state, 1: inhibited operation state
        DISPLAY_RAM_READY   = 0x04, // 0: Display RAM not ready for access, 1: Display RAM ready for access
        CORE_TASK_BUSY      = 0x08, // 0: Core task is idle, 1: Core task is busy (not ready for graphics tasks)
        MEM_READ_FIFO_EMPTY = 0x10, // 0: FIFO of Memory read is not empty, 1: FIFO of memory read is empty
        MEM_READ_FIFO_FULL  = 0x20, // 0: FIFO of memory read is not full, 1: FIFO of memory read is full
        MEM_WRTE_FIFO_EMPTY = 0x40, // 0: FIFO of Memory write is not empty, 1: FIFO of memory write is empty
        MEM_WRTE_FIFO_FULL  = 0x80, // 0: FIFO of memory write is not full, 1: FIFO of memory write is full

    };

    enum LT7680_BTE_ROP : uint8_t {
        BLACKNESS       = 0,    // 0 (Blackness)
        S0_NOR_S1       = 1,    // ~S0・ ~S1 or ~ (S0+S1)
        NOT_S0_AND_S1   = 2,    // ~S0・ S1
        NOT_S0          = 3,    // ~S0
        S0_AND_NOT_S1   = 4,    // S0・ ~S1
        NOT_S1          = 5,    // ~S1
        S0_XOR_S1       = 6,    // S0^S1
        S0_NAND_S1      = 7,    // ~S0+~S1 or ~ (S0・ S1)
        S0_AND_S1       = 8,    // S0・ S1
        S0_XNOR_S1      = 9,    // ~ (S0^S1)
        S1              = 10,   // S1
        NOT_S0_OR_S1    = 11,   // ~S0+S1
        S0              = 12,   // S0
        S0_OR_NOT_S1    = 13,   // S0+~S1
        S0_OR_S1        = 14,   // S0+S1
        WHITENESS       = 15    // 1 (Whiteness)
    };

    enum LT7680_BTE_Operation_Code : uint8_t {
        MCU_WRITE_WITH_ROP                          = 0,
        MEM_COPY_WITH_ROP                           = 2,
        MCU_WRITE_WITH_CHROMA_KEY                   = 4,
        MEM_COPY_WITH_CHROMA_KEY                    = 5,
        PATTERN_FILL_WITH_ROP                       = 6,
        PATTERN_FILL_WITH_CHROMA_KEY                = 7,
        MCU_WRITE_WITH_COLOUR_EXPAND                = 8,
        MCU_WRITE_WITH_COLOUR_EXPAND_AND_CHROMA_KEY = 9,
        MEMORY_COPY_WITH_OPACITY                    = 10,
        MCU_WRITE_WITH_OPACITY                      = 11,
        SOLID_FILL                                  = 12,
        MEM_COPY_WITH_COLOUR_EXPAND                 = 14,
        MEM_COPY_WITH_COLOUR_EXPAND_AND_CHROMA_KEY  = 15
    };

private:
    // Shadow copy of the controller registers to avoid having to implement/use SPI read operations
    // Initial values represent the default values
    struct LT7680Registers {
        uint8_t CCR         = 0x48; // REG[01h] Chip Configuration Register (CCR)
        uint8_t MACR        = 0x00; // REG[02h] Memory Access Control Register (MACR)
        uint8_t ICR         = 0x00; // REG[03h] Input Control Register (ICR) 
        uint8_t MPWCTR      = 0x04; // REG[10h] Main/PIP Window Control Register (MPWCTR)
        uint8_t PIPCDEP     = 0x05; // REG[11h] PIP Window Color Depth Setting (PIPCDEP)
        uint8_t DPCR        = 0x00; // REG[12h] Display Configuration Register (DPCR)
        uint8_t GTCCR       = 0x00; // REG[3Ch] Graphic / Text Cursor Control Register (GTCCR)
        uint8_t AW_COLOR    = 0x00; // REG[5Eh] Color Depth of Canvas & Active Window (AW_COLOR)
        uint8_t PSCLR       = 0x00; // REG[84h] PWM Prescaler Register (PSCLR)
        uint8_t PMUXR       = 0x00; // REG[85h] PWM Clock Mux Register (PMUXR)
        uint8_t PCFGR       = 0x22; // REG[86h] PWM Configuration Register (PCFGR)
        uint8_t BLT_CTRL0   = 0x00; // REG[90h] BTE Function Control Register 0 (BLT_CTRL0)
        uint8_t BLT_COLR    = 0x00; // REG[92h] Source 0/1 & Destination Color Depth (BLT_COLR)
        uint8_t CCR0        = 0x00; // REG[CCh] Character Control Register 0 (CCR0)
        uint8_t CCR1        = 0x00; // REG[CDh] Character Control Register 1 (CCR1)
        uint8_t PMU         = 0x00; // REG[DFh] Power Management Register (PMU)
    } _registers;

    // Same as above, but 16-bit registers (to make life a little easier)
    struct LT7680Registers16 {
        uint16_t CURH   = 0x0000; // REG[60h-5Fh] Graphic Read/Write X-Coordinate Register (CURH)
        uint16_t CURV   = 0x0000; // REG[62h-61h] Graphic Read/Write Y-Coordinate Register (CURV)
    } _registers16;

    struct LT7680BaseMemoryLocations {
        uint32_t main_window = 0;
        uint32_t pip1_window = 0;
        uint32_t pip2_window = 0;
        uint32_t canvas = 0;
        uint32_t bte_s0 = 0;
        uint32_t bte_s1 = 0;
        uint32_t bte_destination = 0;
        uint32_t user_cgram = 0;
        uint32_t user_space_1 = 0;
        uint32_t user_space_2 = 0;
        uint32_t user_space_3 = 0;
        uint32_t user_space_4 = 0;
    } _mem_address;

    struct LT7680UserSpaceConfig {
        uint16_t width;
        uint16_t height;
        LT7680_ColourDepth colourDepth;
    } _user_space_config[4];

    LT7680_Config _config;
    Freq_Config _clocks;
    Arduino_DataBus * _lcd_bus = nullptr;
    LT7680_Rotation _lt7680_rotation = ROTATE_NORMAL;

    uint8_t reverseBits(uint8_t in);
    uint16_t rotate1bitBitmapByLine(uint8_t *data, uint16_t org_w, uint16_t org_h, uint16_t y, uint8_t *output);
    uint16_t rotate1bitBitmapByLine(const uint8_t data[], uint16_t org_w, uint16_t org_h, uint16_t y, uint8_t *output);
    void transformXYforRotation(int16_t logical_x, int16_t logical_y, int16_t *hardware_x, int16_t *hardware_y);
    void transformXYHWforRotation(int16_t logical_x, int16_t logical_y, int16_t logical_width, int16_t logical_height, int16_t *hardware_x, int16_t *hardware_y, int16_t *hardware_width, int16_t *hardware_height);

public:
    void softwareReset(uint16_t delayAfterReset = LT7680_RST_DELAY);
    void hardwareReset(uint16_t delayAfterReset = LT7680_RST_DELAY);
    INLINE bool getStatus(LT7680_Status_Register_Mask v);
    bool isBusy();
    INLINE bool isNormalOperation();
    INLINE bool isDisplayRAMReady();
    INLINE bool isWriteFIFOFull();
    INLINE bool isWriteFIFOEmpty();

    void initialiseLCD_Direct_Bus();
    void initialiseLCD();
    void initialiseLT7680();
    void initialisePLL();
    void initialiseSDRAM();
    void initialiseChipConfiguration();
    void initialiseMemoryAccessControl();
    void initialiseInputControl();
    void initialiseDisplayConfiguration();
    void initialiseCanvas(LT7680_ColourDepth c);

    void initialiseWindowReadyForDrawing(LT7680_WindowSelection wnd, uint32_t memoryStartAddress);

    void setPowerManagementMode(LT7680_Power_Mode mode);
    LT7680_Power_Mode getPowerManagementMode();
    void setPWMPrescaler(uint8_t prescaler_minus1);
    void setPWM0Parameters(uint8_t divisor, uint16_t total_count, uint16_t compare_value, bool inverted = false, bool auto_reload = true, uint8_t dead_zone_length = 0);
    void setPWM1Parameters(uint8_t divisor, uint16_t total_count, uint16_t compare_value, bool inverted = false, bool auto_reload = true);
    void updatePWM0Compare(uint16_t value);
    void updatePWM1Compare(uint16_t value);
    void enablePWM0(bool enable);
    void enablePWM1(bool enable);

    void showTestBars(bool enabled);

    void selectPIPForConfiguration(LT7680_WindowSelection wnd);
    void setPIPWindow(LT7680_WindowSelection wnd, uint16_t hw_x, uint16_t hw_y, uint16_t hw_width, uint16_t hw_height);
    void setPIPWindowLogical(LT7680_WindowSelection wnd, int16_t x, int16_t y, int16_t width, int16_t height);
    void enablePIP(LT7680_WindowSelection wnd, bool enable);

    void setImageColourDepth(LT7680_WindowSelection wnd, LT7680_ColourDepth c);
    void setImageColourDepth(LT7680_WindowSelection wnd, LT7680_ColourDepth_S1 c);
    INLINE LT7680_ColourDepth getImageColourDepth(LT7680_WindowSelection wnd);
    void setStartMemoryAddress(LT7680_WindowSelection wnd, uint32_t addr);
    uint32_t getStartMemoryAddress(LT7680_WindowSelection wnd);
    void setWindowWidth(LT7680_WindowSelection wnd, uint16_t hw_width);
    void setWindowUpperLeftXY(LT7680_WindowSelection wnd, uint16_t hw_x, uint16_t hw_y);
    void setWindowUpperLeftXYLogical(LT7680_WindowSelection wnd, int16_t x, int16_t y, int16_t w = 0, int16_t h = 0);
    void setActiveWindowArea(uint16_t hw_x, uint16_t hw_y, uint16_t hw_width, uint16_t hw_height);
    void setCanvasTo(LT7680_WindowSelection wnd);
    void setMainWindowTo(LT7680_WindowSelection wnd);
    void swapCanvasWithMainWindow(); // Allows for double buffering if Canvas and Main Windows reside in different memory locations

    void setLT7680Rotation(LT7680_Rotation r);

    void prepareForDataWrite(LT7680_DataWriteOperation op = LT7680_DataWriteOperation::DISPLAY_RAM);
    void setGraphicPenPos(uint16_t x, uint16_t y);
    void resetActiveWindowToWholeScreen();

    void setStraightShapeCoordinates(uint16_t x0, uint16_t y0);
    void setStraightShapeCoordinates(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setStraightShapeCoordinates(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void setCurvedShapeParameters(uint16_t major_radius, uint16_t minor_radius);
    void setCurvedShapeParameters(uint16_t major_radius, uint16_t minor_radius, uint16_t centre_x, uint16_t centre_y);
    void startDrawingShape(LT7680_Draw_Shape_Option shp, bool Fill = false, bool PolylineClose = false);

    void setForegroundColour(uint8_t red, uint8_t green, uint8_t blue);
    void setForegroundColour(uint16_t colour);
    void setBackgroundColour(uint8_t red, uint8_t green, uint8_t blue);
    void setBackgroundColour(uint16_t colour);

    void writeColor(uint32_t color, LT7680_ColourDepth destinationCD);
    void writeColor(uint16_t color, LT7680_ColourDepth destinationCD);
    void writeColor(uint8_t color, LT7680_ColourDepth destinationCD);

    void BTE_enable();
    void BTE_setWindowSize(uint16_t width, uint16_t height);
    void BTE_setAlphaBlending(uint8_t alpha);
    void BTE_setOperation(LT7680_BTE_Operation_Code op, uint8_t ROP = 0);

    size_t getImageMemorySize(LT7680_WindowSelection wnd);

    // Extra version to allow for background colour
    void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg_color);

    // Character ROM Functions
    void setCharacterSource(LT7680_CharacterGeneratorSource cgrom);
    void setCharacterHeight(LT7680_CharacterHeight height);
    void setCharacterInternalCGROM(LT7680_CharacterSelection sel);
    void setCharacterFullAlignment(bool enable);
    void setCharacterChromaKeying(bool enable);
    void setCharacterRotation(bool rotateAndFlip);
    void setCharacterWidthEnlargement(uint8_t zoomFactor);
    void setCharacterHeightEnlargement(uint8_t zoomFactor);
    void setCharacterLineGap(uint8_t pixelGap);
    void setCharacterToCharacterGap(uint8_t pixelGap);

    // Text Cursor Functions
    void enterTextMode();
    void setTextMode();
    void setTextCursorPosition(uint16_t x, uint16_t y);
    void writeText(char c);
    void writeText(const char *chars);
    void setTextCursorEnable(bool enable);
    void setTextCursorBlinking(bool enable);
    void setCursonBlinkTime(uint8_t frameCycles);
    void setCursorHorizontalSize(uint8_t pixels);
    void setCursorVerticalSize(uint8_t pixels);

    // Graphics Curson Functions
    void SetGraphicMode();
    void setGraphicCursorEnable(bool enable);
    void setGraphicCursorSet(uint8_t set);
    void setGraphicCursorPosition(uint16_t x, uint16_t y);
    void setGraphicCursonColour0(uint8_t color332);
    void setGraphicCursonColour1(uint8_t color332);


};

#endif // ifndef _ARDUINO_LT7680_H_
