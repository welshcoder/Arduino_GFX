#include "Arduino_LT7680.h"

Arduino_LT7680::Arduino_LT7680(
    Arduino_DataBus *controller, LT7680_Config config, int8_t rst, Arduino_DataBus *lcd_direct)
    : Arduino_TFT(controller, rst, 0, true, config.horizontal_width, config.vertical_height, 0, 0, 0, 0)
{
    _config=config;
    _lcd_bus=lcd_direct;
}

bool Arduino_LT7680::begin(int32_t speed)
{
    _override_datamode = SPI_MODE3;
    if (_override_datamode != GFX_NOT_DEFINED)
    {
        if (!_bus->begin(speed, _override_datamode))
        {
            return false;
        }
    }
    else
    {
        if (!_bus->begin(speed))
        {
            return false;
        }
    }

    tftInit();
    setRotation(_rotation); // apply the setting rotation to the display
    return true;
}

void Arduino_LT7680::tftInit()
{
    // Hardware Reset
    hardwareReset();

    // Software Reset
    softwareReset();

    // Initialise the LCD communication
    initialiseLCD_Direct_Bus();

    // Initialise the LCD using the given initialisation code
    initialiseLCD();

    // Initialise the LT7680 with all the configuration bits to make to show something
    initialiseLT7680();

    // Set up the main window
    initialiseWindowReadyForDrawing(MAIN, 0x0000);

    setCanvasTo(MAIN);
    // Set Canvas width
    setWindowWidth(CANVAS,_config.horizontal_width);
    // Set Active Window location to whole screen
    resetActiveWindowToWholeScreen();

    // At this point, writing image data to the active window will place it on the canvas,
    // which shares the same address as the Main Window, meaning that it will be displayed
    // immediately on write.
    // Now, going to set up PIP1, PIP2 and BTE memory areas just in case they're needed
    uint32_t display_mem_size = getImageMemorySize(MAIN);

    // Set up PIP1
    initialiseWindowReadyForDrawing(PIP1, _mem_address.main_window + display_mem_size);

    // Set up PIP2
    initialiseWindowReadyForDrawing(PIP2, _mem_address.pip1_window + display_mem_size);

    // Set up S0
    initialiseWindowReadyForDrawing(BTE_S0, _mem_address.pip2_window + display_mem_size);

    // Set up S1
    initialiseWindowReadyForDrawing(BTE_S1, _mem_address.bte_s0 + display_mem_size);
}

void Arduino_LT7680::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    if(x < 0)
    {
        w += x;
        x = 0;
    }

    if(y < 0)
    {
        h += y;
        y = 0;
    }
    
    // Set active window to that required
    //setActiveWindowArea(x, y, w, h);

    // Set the position of the graphics pen to origin of active window
    setGraphicPenPos(x, y);

    // Prepare for the colour data write
    prepareForDataWrite(DISPLAY_RAM);
}


void Arduino_LT7680::prepareForDataWrite(LT7680_DataWriteOperation op)
{
    _registers.ICR &= ~(op & 0x03);
    _registers.ICR |= (op & 0x03);

    _bus->sendCommand(LT7680_ICR);
    _bus->sendData(_registers.ICR);
    _bus->sendCommand(LT7680_MRWDP);
}

void Arduino_LT7680::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
{
    setGraphicPenPos(x, y);
    prepareForDataWrite();
    writeColor(color);   
}

/**************************************************************************/
/*!
  @brief  Write a line.  
  @param  x0      Start point x coordinate
  @param  y0      Start point y coordinate
  @param  x1      End point x coordinate
  @param  y1      End point y coordinate
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::writeSlashLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    // int16_t hw_x0, hw_y0, hw_x1, hw_y1;
    // transformXYforRotation(x0, y0, &hw_x0, &hw_y0);
    // transformXYforRotation(x1, y1, &hw_x1, &hw_y1);

    setForegroundColour(color);
    setStraightShapeCoordinates(x0, y0, x1, y1);
    startDrawingShape(DRAW_LINE,false,false);
}

/**************************************************************************/
/*!
  @brief  Write a perfectly vertical line, overwrite in subclasses if startWrite is defined!
  @param  x       Top-most x coordinate
  @param  y       Top-most y coordinate
  @param  h       Height in pixels
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    writeSlashLine(x, y, x, y + h, color);
}

/**************************************************************************/
/*!
  @brief  Write a perfectly horizontal line, overwrite in subclasses if startWrite is defined!
  @param  x       Left-most x coordinate
  @param  y       Left-most y coordinate
  @param  w       Width in pixels
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    writeSlashLine(x, y, x + w, y, color);
}

/**************************************************************************/
/*!
  @brief  Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  w       Width in pixels
  @param  h       Height in pixels
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    //setActiveWindowArea(hw_x, hw_y, hw_w, hw_h);
    setStraightShapeCoordinates(hw_x, hw_y, hw_x + hw_w, hw_y + hw_h);
    setForegroundColour(color);
    startDrawingShape(DRAW_RECT,true);

    // Return the active window to the whole screen
    //setActiveWindowArea(0, 0, _config.horizontal_width, _config.vertical_height);
}

void Arduino_LT7680::writeColor(uint16_t color)
{
    writeColor(color, getImageColourDepth(CANVAS));
}

void Arduino_LT7680::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x, hw_y, hw_x + hw_w, hw_y + hw_h);
    startDrawingShape(DRAW_RECT, true);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_LT7680::setRotation(uint8_t r)
{
    // Going to do image rotation on the LT7680
    Arduino_TFT::setRotation(r);
    setLT7680Rotation((LT7680_Rotation)(r));
}


/**************************************************************************/
/*!
  @brief  Invert the display (ideally using built-in hardware command)
  @param  i   True if you want to invert, false to make 'normal'
*/
/**************************************************************************/
void Arduino_LT7680::invertDisplay(bool invert)
{
    if(invert)
        _registers.DPCR |= 0x08;
    else
        _registers.DPCR &= ~0x08;

    while(isBusy());
    _bus->sendCommand(LT7680_DPCR);
    _bus->sendData(_registers.DPCR);
}

/**************************************************************************/
/*!
  @brief  Turn on display after turned off
*/
/**************************************************************************/
void Arduino_LT7680::displayOn(void)
{
    _registers.DPCR |= 0x40;

    while(isBusy());
    _bus->sendCommand(LT7680_DPCR);
    _bus->sendData(_registers.DPCR);
}

/**************************************************************************/
/*!
  @brief  Turn off display
*/
/**************************************************************************/
void Arduino_LT7680::displayOff(void)
{
    _registers.DPCR &= ~0x40;

    while(isBusy());
    _bus->sendCommand(LT7680_DPCR);
    _bus->sendData(_registers.DPCR);
}

/**************************************************************************/
/*!
  @brief  Fill the screen completely with one color. Update in subclasses if desired!
  @param  color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::fillScreen(uint16_t color)
{
    setForegroundColour(color);
    setStraightShapeCoordinates(0, 0, _config.horizontal_width, _config.vertical_height);
    startDrawingShape(DRAW_RECT,true);
}

/**************************************************************************/
/*!
  @brief  Draw a line
  @param  x0      Start point x coordinate
  @param  y0      Start point y coordinate
  @param  x1      End point x coordinate
  @param  y1      End point y coordinate
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t hw_x0, hw_y0, hw_x1, hw_y1;
    transformXYforRotation(x0, y0, &hw_x0, &hw_y0);
    transformXYforRotation(x1, y1, &hw_x1, &hw_y1);

    writeLine(hw_x0, hw_y0, hw_x1, hw_y1, color);
}

/**************************************************************************/
/*!
  @brief  Write a line. Check straight or slash line and call corresponding function
  @param  x0      Start point x coordinate
  @param  y0      Start point y coordinate
  @param  x1      End point x coordinate
  @param  y1      End point y coordinate
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    writeSlashLine(x0, y0, x1, y1, color);
}

/**************************************************************************/
/*!
  @brief  Draw a circle outline
  @param  x       Center-point x coordinate
  @param  y       Center-point y coordinate
  @param  r       Radius of circle
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    int16_t hw_x, hw_y;
    transformXYforRotation(x, y, &hw_x, &hw_y);

    setForegroundColour(color);
    setCurvedShapeParameters(r, r, hw_x, hw_y);
    startDrawingShape(DRAW_ELLIPSE);
}

/**************************************************************************/
/*!
  @brief  Draw an ellipse outline
  @param  x       Center-point x coordinate
  @param  y       Center-point y coordinate
  @param  rx      radius of x coordinate
  @param  ry      radius of y coordinate
  @param  start   degree of ellipse start
  @param  end     degree of ellipse end
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
    int16_t hw_x, hw_y;
    transformXYforRotation(x, y, &hw_x, &hw_y);

    setForegroundColour(color);
    setCurvedShapeParameters(rx, ry, hw_x, hw_y);
    startDrawingShape(DRAW_ELLIPSE);
}

/**************************************************************************/
/*!
  @brief  Draw a circle with filled color
  @param  x       Center-point x coordinate
  @param  y       Center-point y coordinate
  @param  r       Radius of circle
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t hw_x0, hw_y0;
    transformXYforRotation(x0, y0, &hw_x0, &hw_y0);

    setForegroundColour(color);
    setCurvedShapeParameters(r, r, hw_x0, hw_y0);
    startDrawingShape(DRAW_ELLIPSE,true);
}

/**************************************************************************/
/*!
  @brief  Draw an ellipse with filled color
  @param  x       Center-point x coordinate
  @param  y       Center-point y coordinate
  @param  rx      radius of x coordinate
  @param  ry      radius of y coordinate
  @param  start   degree of ellipse start
  @param  end     degree of ellipse end
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
    int16_t hw_x, hw_y;
    transformXYforRotation(x, y, &hw_x, &hw_y);

    setForegroundColour(color);
    setCurvedShapeParameters(rx, ry, hw_x, hw_y);
    startDrawingShape(DRAW_ELLIPSE,true);
}

/**************************************************************************/
/*!
  @brief  Draw a triangle with no fill color
  @param  x0      Vertex #0 x coordinate
  @param  y0      Vertex #0 y coordinate
  @param  x1      Vertex #1 x coordinate
  @param  y1      Vertex #1 y coordinate
  @param  x2      Vertex #2 x coordinate
  @param  y2      Vertex #2 y coordinate
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    int16_t hw_x0, hw_y0, hw_x1, hw_y1, hw_x2, hw_y2;
    transformXYforRotation(x0, y0, &hw_x0, &hw_y0);
    transformXYforRotation(x1, y1, &hw_x1, &hw_y1);
    transformXYforRotation(x2, y2, &hw_x2, &hw_y2);

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x0, hw_y0, hw_x1, hw_y1, hw_x2, hw_y2);
    startDrawingShape(DRAW_TRIANGLE);
}

/**************************************************************************/
/*!
  @brief  Draw a triangle with color-fill
  @param  x0      Vertex #0 x coordinate
  @param  y0      Vertex #0 y coordinate
  @param  x1      Vertex #1 x coordinate
  @param  y1      Vertex #1 y coordinate
  @param  x2      Vertex #2 x coordinate
  @param  y2      Vertex #2 y coordinate
  @param  color   16-bit 5-6-5 Color to fill/draw with
*/
/**************************************************************************/
void Arduino_LT7680::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    int16_t hw_x0, hw_y0, hw_x1, hw_y1, hw_x2, hw_y2;
    transformXYforRotation(x0, y0, &hw_x0, &hw_y0);
    transformXYforRotation(x1, y1, &hw_x1, &hw_y1);
    transformXYforRotation(x2, y2, &hw_x2, &hw_y2);

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x0, hw_y0, hw_x1, hw_y1, hw_x2, hw_y2);
    startDrawingShape(DRAW_TRIANGLE, true);
}

/**************************************************************************/
/*!
  @brief  Draw a rounded rectangle with no fill color
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  w       Width in pixels
  @param  h       Height in pixels
  @param  r       Radius of corner rounding
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    int16_t hw_x0, hw_y0, hw_w, hw_h;
    transformXYHWforRotation(x0, y0, w, h, &hw_x0, &hw_y0, &hw_w, &hw_h);

    SERIAL_PRINTF("drawRoundRect: x: %d, y: %d, w: %d, h: %d, r: %d, colour: %04x\n", hw_x0, hw_y0, hw_w, hw_h, radius, color);    

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x0, hw_y0, hw_x0 + hw_w, hw_y0 + hw_h);
    setCurvedShapeParameters(radius, radius);
    startDrawingShape(DRAW_RND_RECT);
}

/**************************************************************************/
/*!
  @brief  Draw a rounded rectangle with fill color
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  w       Width in pixels
  @param  h       Height in pixels
  @param  r       Radius of corner rounding
  @param  color   16-bit 5-6-5 Color to draw/fill with
*/
/**************************************************************************/
void Arduino_LT7680::fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    int16_t hw_x0, hw_y0, hw_w, hw_h;
    transformXYHWforRotation(x0, y0, w, h, &hw_x0, &hw_y0, &hw_w, &hw_h);

    SERIAL_PRINTF("fillRoundRect: x: %d, y: %d, w: %d, h: %d, r: %d, colour: %04x\n", hw_x0, hw_y0, hw_w, hw_h, radius, color);    

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x0, hw_y0, hw_x0 + hw_w, hw_y0 + hw_h);
    setCurvedShapeParameters(radius, radius);
    startDrawingShape(DRAW_RND_RECT, true);
}

/**************************************************************************/
/*!
  @brief  Draw a rectangle with no fill color
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  w       Width in pixels
  @param  h       Height in pixels
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setForegroundColour(color);
    setStraightShapeCoordinates(hw_x, hw_y, hw_x + hw_w, hw_y + hw_h);
    startDrawingShape(DRAW_RECT);
}

/**************************************************************************/
/*!
  @brief  Fill a rectangle completely with one color. Update in subclasses if desired!
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  w       Width in pixels
  @param  h       Height in pixels
  @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_LT7680::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    writeFillRect(x, y, w, h, color);
}


/**************************************************************************/
/*!
  @brief  Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with 16-bit color bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_LT7680::draw16bitRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h)
{
    LT7680_ColourDepth cd = getImageColourDepth(CANVAS);

    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setActiveWindowArea(hw_x, hw_y, hw_w, hw_h);
    setGraphicPenPos(hw_x, hw_y);
    prepareForDataWrite(DISPLAY_RAM);
    for(int16_t i = 0; i < h; i++)
    {
        int16_t temp=w * ((_lt7680_rotation==ROTATE_180 || _lt7680_rotation == ROTATE_RIGHT_90)?(h-i):i);

        for(int16_t j = 0; j < w ; j++)
        {
            writeColor(pgm_read_word(&bitmap[temp + j]), cd);
        }
        while(!isWriteFIFOEmpty());
    }

    resetActiveWindowToWholeScreen();
}

/**************************************************************************/
/*!
  @brief  Draw a RAM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with 16-bit color bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_LT7680::draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
    LT7680_ColourDepth cd = getImageColourDepth(CANVAS);

    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setActiveWindowArea(hw_x, hw_y, hw_w, hw_h);
    setGraphicPenPos(hw_x, hw_y);
    prepareForDataWrite(DISPLAY_RAM);
    for(int16_t i = 0; i < h; i++)
    {
        int16_t temp=w * ((_lt7680_rotation==ROTATE_180 || _lt7680_rotation == ROTATE_RIGHT_90)?(h-i):i);

        for(int16_t j = 0; j < w ; j++)
        {
            writeColor(bitmap[temp + j], cd);
        }
        while(!isWriteFIFOEmpty());
    }

    resetActiveWindowToWholeScreen();
}

/**************************************************************************/
/*!
  @brief  Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with monochrome bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
  @param  color   16-bit 5-6-5 Color to draw pixels with
  @param  bg      16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void Arduino_LT7680::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setStartMemoryAddress(BTE_DEST,0);
    setWindowWidth(BTE_DEST,_config.horizontal_width);
    setWindowUpperLeftXY(BTE_DEST, hw_x, hw_y);

    setImageColourDepth(BTE_DEST,_config.mem_colour_depth);

    BTE_setWindowSize(hw_w, hw_h);
    setForegroundColour(color);
    setBackgroundColour(bg);

    BTE_setOperation(MCU_WRITE_WITH_COLOUR_EXPAND,7);
    BTE_enable();

    _bus->sendCommand(LT7680_MRWDP);

    int16_t bytesPerRow = (hw_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t lineData[bytesPerRow];

    for(uint16_t j=0; j < hw_h ; j++)
    {
        uint16_t row = rotate1bitBitmapByLine(bitmap, w, h, j, lineData);
        for(uint16_t i = 0; i < row ; i++)
            _bus->sendData(lineData[i]);

        while(!isWriteFIFOEmpty());
    }
}

/**************************************************************************/
/*!
  @brief  Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with monochrome bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
  @param  color   16-bit 5-6-5 Color to draw pixels with
  @param  bg      16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void Arduino_LT7680::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
{

    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setStartMemoryAddress(BTE_DEST,0);
    setWindowWidth(BTE_DEST,_config.horizontal_width);
    setWindowUpperLeftXY(BTE_DEST, hw_x, hw_y);

    setImageColourDepth(BTE_DEST,_config.mem_colour_depth);

    BTE_setWindowSize(hw_w, hw_h);
    setForegroundColour(color);
    setBackgroundColour(bg);

    BTE_setOperation(MCU_WRITE_WITH_COLOUR_EXPAND,7);
    BTE_enable();

    _bus->sendCommand(LT7680_MRWDP);

    int16_t bytesPerRow = (hw_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t lineData[bytesPerRow];

    for(uint16_t j=0; j < hw_h ; j++)
    {
        uint16_t row = rotate1bitBitmapByLine(bitmap, w, h, j, lineData);
        for(uint16_t i = 0; i < row ; i++)
            _bus->sendData(lineData[i]);

        while(!isWriteFIFOEmpty());
    }
}

/**************************************************************************/
/*!
  @brief  Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with monochrome bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setStartMemoryAddress(BTE_DEST,0);
    setWindowWidth(BTE_DEST,_config.horizontal_width);
    setWindowUpperLeftXY(BTE_DEST, hw_x, hw_y);

    setImageColourDepth(BTE_DEST,_config.mem_colour_depth);

    BTE_setWindowSize(hw_w, hw_h);
    setForegroundColour(color);
   
    BTE_setOperation(MCU_WRITE_WITH_COLOUR_EXPAND_AND_CHROMA_KEY,7);
    BTE_enable();

    _bus->sendCommand(LT7680_MRWDP);

    int16_t bytesPerRow = (hw_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t lineData[bytesPerRow];

    for(uint16_t j=0; j < hw_h ; j++)
    {
        uint16_t row = rotate1bitBitmapByLine(bitmap, w, h, j, lineData);
        for(uint16_t i = 0; i < row ; i++)
            _bus->sendData(lineData[i]);

        while(!isWriteFIFOEmpty());
    }
}

/**************************************************************************/
/*!
  @brief  Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with monochrome bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
  @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_LT7680::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
    int16_t hw_x, hw_y, hw_w, hw_h;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);

    setStartMemoryAddress(BTE_DEST,0);
    setWindowWidth(BTE_DEST,_config.horizontal_width);
    setWindowUpperLeftXY(BTE_DEST, hw_x, hw_y);

    setImageColourDepth(BTE_DEST,_config.mem_colour_depth);

    BTE_setWindowSize(hw_w, hw_h);
    setForegroundColour(color);
   
    BTE_setOperation(MCU_WRITE_WITH_COLOUR_EXPAND_AND_CHROMA_KEY,7);
    BTE_enable();

    _bus->sendCommand(LT7680_MRWDP);

    int16_t bytesPerRow = (hw_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t lineData[bytesPerRow];

    for(uint16_t j=0; j < hw_h ; j++)
    {
        uint16_t row = rotate1bitBitmapByLine(bitmap, w, h, j, lineData);
        for(uint16_t i = 0; i < row ; i++)
            _bus->sendData(lineData[i]);
            

        while(!isWriteFIFOEmpty());
    }

}

/**************************************************************************/
/*!
   @brief   Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP.
    Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
    C Array can be directly used with this function.
    There is no RAM-resident version of this function; if generating bitmaps
    in RAM, use the format defined by drawBitmap() and call that instead.
  @param  x       Top left corner x coordinate
  @param  y       Top left corner y coordinate
  @param  bitmap  byte array with monochrome bitmap
  @param  w       Width of bitmap in pixels
  @param  h       Height of bitmap in pixels
  @param  color   16-bit 5-6-5 Color to draw pixels with
*/
/**************************************************************************/
void Arduino_LT7680::drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t bytesPerRow = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t reversedInput[h*bytesPerRow];
    for(uint16_t i=0;i<h*bytesPerRow;i++)
        reversedInput[i]=reverseBits(pgm_read_byte(&bitmap[i]));

    drawBitmap(x, y, reversedInput, w, h, color);
}



void Arduino_LT7680::drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg_color)
{
    int16_t bytesPerRow = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t reversedInput[h*bytesPerRow];
    for(uint16_t i=0;i<h*bytesPerRow;i++)
        reversedInput[i]=reverseBits(pgm_read_byte(&bitmap[i]));

    drawBitmap(x, y, reversedInput, w, h, color, bg_color);
}


/*
 *
 * The methods below are private to help implement the overridden methods above
 * 
 */

void Arduino_LT7680::softwareReset(uint16_t delayAfterReset)
{
    // Software Reset
    while(isBusy());
    _bus->sendCommand(LT7680_SRR);
    _bus->sendData(0x02);
    delay(delayAfterReset);
}

void Arduino_LT7680::hardwareReset(uint16_t delayAfterReset)
{
    if (_rst == GFX_NOT_DEFINED)
        return;

    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(delayAfterReset);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(delayAfterReset);
}

bool Arduino_LT7680::getStatus(LT7680_Status_Register_Mask v)
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return _bus->receive16(0x4000) & v;
#else
    return 0x54 & v; // Everthing is good because we don't know any better.
#endif
}

bool Arduino_LT7680::isBusy()
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return getStatus(CORE_TASK_BUSY);
#else
    delayMicroseconds(LT7680_PROCESSING_TIME_US);
    return false;
#endif
}

bool Arduino_LT7680::isNormalOperation()
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return !getStatus(OPERATION_MODE);
#else
    return true;
#endif
}

bool Arduino_LT7680::isDisplayRAMReady()
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return getStatus(DISPLAY_RAM_READY);
#else
    return true;
#endif
}

bool Arduino_LT7680::isWriteFIFOFull()
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return getStatus(MEM_WRTE_FIFO_FULL);
#else
    return false;
#endif
}

bool Arduino_LT7680::isWriteFIFOEmpty()
{
#if defined(ARDUINO_GFX_INC_READ_OPERATIONS)
    return getStatus(MEM_WRTE_FIFO_EMPTY);
#else
    return true;
#endif
}

void Arduino_LT7680::initialiseLCD_Direct_Bus()
{
    if(_lcd_bus==nullptr)
        return;

    _lcd_bus->begin(_config.lcd_init.speed,_config.lcd_init.mode);
}

void Arduino_LT7680::initialiseLCD()
{
    if(_lcd_bus==nullptr)
        return;

    _lcd_bus->batchOperation(_config.lcd_init.initial_operations,_config.lcd_init.init_op_length);
}

void Arduino_LT7680::initialiseLT7680()
{
    initialisePLL();
    initialiseSDRAM();
    initialiseChipConfiguration();
    initialiseMemoryAccessControl();
    initialiseInputControl();
    initialiseDisplayConfiguration();
    initialiseCanvas(_config.mem_colour_depth);
}


void Arduino_LT7680::initialisePLL()
{
    PLL_Config CPLL, MPLL, PPLL;

    switch(_config.clock_config_sel)
    {
        case PLL_PARAMETERS:
            CPLL = {
                .OD = (uint8_t)((_config.PLL.CPLL.OD - 1) & 0x03), // Only 2 bits are allowed
                .R = (uint8_t)(_config.PLL.CPLL.R & 0x1F), // Only 5 bits are allowed
                .N = (uint16_t)(_config.PLL.CPLL.N & 0x01FF) // Only 9 bits are allowed
            };
            // Fout = XI * (N / R) / OD, where XI = 10 MHz
            _clocks.CCLK = 10 * (CPLL.N / CPLL.R) / (CPLL.OD + 1);

            MPLL = {
                .OD = (uint8_t)((_config.PLL.MPLL.OD - 1) & 0x03), // Only 2 bits are allowed
                .R = (uint8_t)(_config.PLL.MPLL.R & 0x1F), // Only 5 bits are allowed
                .N = (uint16_t)(_config.PLL.MPLL.N & 0x01FF) // Only 9 bits are allowed
            };
            _clocks.MCLK = 10 * (MPLL.N / MPLL.R) / (MPLL.OD + 1);
 
            PPLL = {
                .OD = (uint8_t)((_config.PLL.PPLL.OD - 1) & 0x03), // Only 2 bits are allowed
                .R = (uint8_t)(_config.PLL.PPLL.R & 0x1F), // Only 5 bits are allowed
                .N = (uint16_t)(_config.PLL.PPLL.N & 0x01FF) // Only 9 bits are allowed
            };
            _clocks.PCLK = 10 * (PPLL.N / PPLL.R) / (PPLL.OD + 1);
            break;
        default:
        case FREQUENCY_MHZ:
            CPLL = {
                .OD = 2 - 1,
                .R = 5,
                .N = (uint16_t)(_config.FreqMHz.CCLK & 0x01FF) // Only 9 bits are allowed
            };
            MPLL = {
                .OD = 2 - 1,
                .R = 5,
                .N = (uint16_t)(_config.FreqMHz.MCLK & 0x01FF) // Only 9 bits are allowed
            };
            PPLL = {
                .OD = 2 - 1,
                .R = 5,
                .N = (uint16_t)(_config.FreqMHz.PCLK & 0x01FF) // Only 9 bits are allowed
            };
            _clocks=_config.FreqMHz;
    }

    while(isBusy());

    // Set PLL settings for CCLK
    _bus->sendCommand(LT7680_CPLLC1);
    _bus->sendData((CPLL.OD << 6) | (CPLL.R << 1) | ((CPLL.N >> 8) & 0x01));
    _bus->sendCommand(LT7680_CPLLC2);
    _bus->sendData(CPLL.N & 0xFF);

    // Set PLL settings for MCLK
    _bus->sendCommand(LT7680_MPLLC1);
    _bus->sendData((MPLL.OD << 6) | (MPLL.R << 1) | ((MPLL.N >> 8) & 0x01));
    _bus->sendCommand(LT7680_MPLLC2);
    _bus->sendData(MPLL.N & 0xFF);

    // Set PLL settings for PCLK
    _bus->sendCommand(LT7680_PPLLC1);
    _bus->sendData((PPLL.OD << 6) | (PPLL.R << 1) | ((PPLL.N >> 8) & 0x01));
    _bus->sendCommand(LT7680_PPLLC2);
    _bus->sendData(PPLL.N & 0xFF);

    // Tell the LT7680 to reconfigure the PLL frequency
    _bus->sendCommand(LT7680_SRR);
    delayMicroseconds(1);
    _bus->sendData(0x80);
    delay(1);
}

void Arduino_LT7680::initialiseSDRAM()
{
    while(isBusy());

    _bus->sendCommand(LT7680_SDRAR);
    _bus->sendData(LT7680_SDRAR_SETTING);

    // Set SDRAM CAS latency to 3
    _bus->sendCommand(LT7680_SDRMD);
    _bus->sendData(0x03); 

    // Not sure where these values have come from - need to check
    uint16_t sdram_itv = (64000000 / 8192) / (1000/_clocks.MCLK);
    sdram_itv -= 2;
    _bus->sendCommand(LT7680_SDR_REF_LO);
    _bus->sendData(sdram_itv & 0xFF);
    _bus->sendCommand(LT7680_SDR_REF_HI);
    _bus->sendData((sdram_itv >> 8) & 0xFF);

    // Start SDRAM initialisation procedure
    _bus->sendCommand(LT7680_SDRCR);
    _bus->sendData(0x01);

    while(!isDisplayRAMReady());
}

void Arduino_LT7680::initialiseChipConfiguration()
{
    // Set the TFT Panel interface colour data width
    _registers.CCR &= ~((_config.colour_width & 0x03) << 3);
    _registers.CCR |=  ((_config.colour_width & 0x03) << 3);

    // keypad-scan disable
    _registers.CCR &= ~0x20;

    // I2C Master Interface Disable
    _registers.CCR &= ~0x04;

    // Serial Flash or SPI Interface Disable
    _registers.CCR &= ~0x02;

    // Host Data Bus Width Selection to 8-bit
    _registers.CCR &= ~0x01;

    // Set the register
    while(isBusy());
    _bus->sendCommand(LT7680_CCR);
    _bus->sendData(_registers.CCR);
}

void Arduino_LT7680::initialiseMemoryAccessControl()
{
    // Host R/W Image Data Format to direct write
    _registers.MACR &= ~0xC0;

    // Set Host Read Memory Left to Right, Top to Down
    _registers.MACR &= ~0x30;

    // Set the register
    while(isBusy());
    _bus->sendCommand(LT7680_MACR);
    _bus->sendData(_registers.MACR);
}

void Arduino_LT7680::initialiseInputControl()
{
    // Set to graphic mode
    _registers.ICR &= ~0x04;

    // Select Display RAM for destination
    _registers.ICR &= ~0x03;

    // Set the register
    while(isBusy());
    _bus->sendCommand(LT7680_ICR);
    _bus->sendData(_registers.ICR);
}

void Arduino_LT7680::initialiseDisplayConfiguration()
{
    // Set HSCAN from left to right (this seems to be undocumented, but is done in the example Arduino code)
    _registers.DPCR &= ~0x10;

    // Set vertical scan direction to be top to bottom
    _registers.DPCR &= ~0x08;

    // Set colour data output sequence to RGB
    _registers.DPCR &= ~0x07;

    // Disable test colour bar
    _registers.DPCR &= ~0x20;
//    _registers.DPCR |= 0x20;

    // Set the active edge of PCLK
    if(_config.PCLK_active_edge==RISING_EDGE)
        _registers.DPCR &= ~0x80;
    else
        _registers.DPCR |= 0x80;

    // PCSR Register
    uint8_t reg_13 = 3; // Default value

    // Set HSYNC Polarity
    reg_13 |= ((_config.HSYNC_polarity & 0x01) << 7);

    // Set VSYNC Polarity
    reg_13 |= ((_config.VSYNC_polarity & 0x01) << 6);

    // Set DE Polarity (it's inverted compared to the other settings!)
    reg_13 |= (((~_config.DE_polarity) & 0x01) << 5);

    // HDWR and HDWFTR Registers (horizontal display width)
    uint8_t reg_14 = (_config.horizontal_width > 7)?((_config.horizontal_width / 8) - 1):0;
    uint8_t reg_15 = _config.horizontal_width & 0x0F;

    // HNDR and HNDFTR Registers (horizontal non-display period, i.e. HSYNC back porch)
    uint8_t reg_16 = ((_config.HSYNC_back_porch > 7)?((_config.HSYNC_back_porch / 8) - 1):0) & 0x1F;
    uint8_t reg_17 = _config.HSYNC_back_porch & 0x0F;

    // HSTR Register (HSYNC Start Position, i.e. HSYNC Front Porch)
    uint8_t reg_18 = ((_config.HSYNC_front_porch > 7)?((_config.HSYNC_front_porch / 8) - 1):0) & 0x1F;

    // HPWR Register (HSYNC Pulse Width)
    uint8_t reg_19 = ((_config.HSYNC_pulse_width > 7)?((_config.HSYNC_pulse_width / 8) - 1):0) & 0x1F;

    // VDHR Registers (vertical display height)
    uint8_t reg_1a = (_config.vertical_height - 1) & 0xFF;
    uint8_t reg_1b = ((_config.vertical_height - 1) >> 8) & 0x07;

    // VNDR Register (vertical non-display period, i.e. VSYNC back porch)
    uint8_t reg_1c = (_config.VSYNC_back_porch > 0)?((_config.VSYNC_back_porch - 1) & 0xFF):0;
    uint8_t reg_1d = (_config.VSYNC_back_porch >> 8) & 0x03;

    // VSTR Register (VSYNC Start Position, i.e. VSYNC Front Porch)
    uint8_t reg_1e = (_config.VSYNC_front_porch > 0)?(_config.VSYNC_front_porch - 1):0;

    // VPWR Register (VSYNC Pulse Width)
    uint8_t reg_1f = ((_config.VSYNC_pulse_width > 0)?(_config.VSYNC_pulse_width - 1):0) & 0x3F;

    // Send the values to the chip
    while(isBusy());
    _bus->sendCommand(LT7680_DPCR);
    _bus->sendData(_registers.DPCR);
    _bus->sendCommand(LT7680_PCSR);
    _bus->sendData(reg_13);
    _bus->sendCommand(LT7680_HDWR);
    _bus->sendData(reg_14);
    _bus->sendCommand(LT7680_HDWFTR);
    _bus->sendData(reg_15);
    _bus->sendCommand(LT7680_HNDR);
    _bus->sendData(reg_16);
    _bus->sendCommand(LT7680_HNDFTR);
    _bus->sendData(reg_17);
    _bus->sendCommand(LT7680_HSTR);
    _bus->sendData(reg_18);
    _bus->sendCommand(LT7680_HPWR);
    _bus->sendData(reg_19);
    _bus->sendCommand(LT7680_VDHR_LO);
    _bus->sendData(reg_1a);
    _bus->sendCommand(LT7680_VDHR_HI);
    _bus->sendData(reg_1b);
    _bus->sendCommand(LT7680_VNDR_LO);
    _bus->sendData(reg_1c);
    _bus->sendCommand(LT7680_VNDR_HI);
    _bus->sendData(reg_1d);
    _bus->sendCommand(LT7680_VSTR);
    _bus->sendData(reg_1e);
    _bus->sendCommand(LT7680_VPWR);
    _bus->sendData(reg_1f);
}

void Arduino_LT7680::initialiseCanvas(LT7680_ColourDepth c)
{
    // Set canvas addressing mode to block (X-Y coordinate addressing)
    _registers.AW_COLOR &= ~0x04;

    // Set canvas colour depth
    setImageColourDepth(CANVAS,c);
}

void Arduino_LT7680::initialiseWindowReadyForDrawing(LT7680_WindowSelection wnd, uint32_t memoryStartAddress)
{
    setImageColourDepth(wnd, _config.mem_colour_depth);
    setStartMemoryAddress(wnd, memoryStartAddress);
    setWindowWidth(wnd, _config.horizontal_width);
    setWindowUpperLeftXY(wnd, 0, 0);

}

void Arduino_LT7680::setPowerManagementMode(LT7680_Power_Mode mode)
{
    _registers.PMU = (mode==NORMAL)?0x80:0x00;
    _registers.PMU |= (mode & 3);

    while(isBusy());
    _bus->sendCommand(LT7680_PMU);
    _bus->sendData(_registers.PMU);
    delay(1);
}

Arduino_LT7680::LT7680_Power_Mode Arduino_LT7680::getPowerManagementMode()
{
    return (LT7680_Power_Mode)(_registers.PMU & 0x03);
}

void Arduino_LT7680::setPWMPrescaler(uint8_t prescaler_minus1)
{
    while(isBusy());
    _bus->sendCommand(LT7680_PSCLR);
    _bus->sendData(prescaler_minus1);
}

void Arduino_LT7680::setPWM0Parameters(uint8_t divisor, uint16_t total_count, uint16_t compare_value, bool inverted, bool auto_reload, uint8_t dead_zone_length)
{
    // 00b: 1, 01b: 1/2, 10b: 1/4, 11b: 1/8 
    divisor &= 0x03;
    // Add PWM0 Function control after bitshifting
    divisor = (divisor << 4) | 0x02;

    _registers.PMUXR &= ~divisor;
    _registers.PMUXR |= divisor;
    
    uint8_t val = (inverted ? 0x04 : 0) | (auto_reload ? 0x02 : 0) | (dead_zone_length>0 ? 0x08 : 0);
    _registers.PCFGR &= ~val;
    _registers.PCFGR |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_PMUXR);
    _bus->sendData(_registers.PMUXR);
    _bus->sendCommand(LT7680_PCFGR);
    _bus->sendData(_registers.PCFGR);
    _bus->sendCommand(LT7680_DZ_LENGTH);
    _bus->sendData(dead_zone_length);
    _bus->sendCommand(LT7680_TCNTB0_LO);
    _bus->sendData(total_count & 0xFF);
    _bus->sendCommand(LT7680_TCNTB0_HI);
    _bus->sendData((total_count >> 8) & 0xFF);
    updatePWM0Compare(compare_value);
}

void Arduino_LT7680::setPWM1Parameters(uint8_t divisor, uint16_t total_count, uint16_t compare_value, bool inverted, bool auto_reload)
{
    // 00b: 1, 01b: 1/2, 10b: 1/4, 11b: 1/8 
    divisor &= 0x03;
    // Add PWM0 Function control after bitshifting
    divisor = (divisor << 6) | 0x08;

    _registers.PMUXR &= ~divisor;
    _registers.PMUXR |= divisor;
    
    uint8_t val = (inverted ? 0x40 : 0) | (auto_reload ? 0x20 : 0);
    _registers.PCFGR &= ~val;
    _registers.PCFGR |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_PMUXR);
    _bus->sendData(_registers.PMUXR);
    _bus->sendCommand(LT7680_PCFGR);
    _bus->sendData(_registers.PCFGR);
    _bus->sendCommand(LT7680_TCNTB1_LO);
    _bus->sendData(total_count & 0xFF);
    _bus->sendCommand(LT7680_TCNTB1_HI);
    _bus->sendData((total_count >> 8) & 0xFF);
    updatePWM1Compare(compare_value);
}

void Arduino_LT7680::updatePWM0Compare(uint16_t value)
{
    while(isBusy());
    _bus->sendCommand(LT7680_TCMPB0_LO);
    _bus->sendData(value & 0xFF);
    _bus->sendCommand(LT7680_TCMPB0_HI);
    _bus->sendData((value >> 8) & 0xFF);
}

void Arduino_LT7680::updatePWM1Compare(uint16_t value)
{
    while(isBusy());
    _bus->sendCommand(LT7680_TCMPB1_LO);
    _bus->sendData(value & 0xFF);
    _bus->sendCommand(LT7680_TCMPB1_HI);
    _bus->sendData((value >> 8) & 0xFF);
}

void Arduino_LT7680::enablePWM0(bool enable)
{
    if(enable)
        _registers.PCFGR |= 0x01;
    else
        _registers.PCFGR &= ~0x01;

    while(isBusy());
    _bus->sendCommand(LT7680_PCFGR);
    _bus->sendData(_registers.PCFGR);
}

void Arduino_LT7680::enablePWM1(bool enable)
{
    if(enable)
        _registers.PCFGR |= 0x10;
    else
        _registers.PCFGR &= ~0x10;

    while(isBusy());
    _bus->sendCommand(LT7680_PCFGR);
    _bus->sendData(_registers.PCFGR);
}

void Arduino_LT7680::showTestBars(bool enabled)
{
    if(enabled)
        _registers.DPCR |= 0x40;
    else
        _registers.DPCR &= ~0x40;

    while(isBusy());
    _bus->sendCommand(LT7680_DPCR);
    _bus->sendData(_registers.DPCR);
}

void Arduino_LT7680::setImageColourDepth(LT7680_WindowSelection wnd, LT7680_ColourDepth c)
{
    while(isBusy());

    switch(wnd)
    {
        case MAIN:
            _registers.MPWCTR &= ~((c & 0x03) << 2);
            _registers.MPWCTR |= ((c & 0x03) << 2);
            _bus->sendCommand(LT7680_MPWCTR);
            _bus->sendData(_registers.MPWCTR);
            break;
        case PIP1:
            _registers.PIPCDEP &= ~((c & 0x03) << 2);
            _registers.PIPCDEP |= ((c & 0x03) << 2);
            _bus->sendCommand(LT7680_PIPCDEP);
            _bus->sendData(_registers.PIPCDEP);
            break;
        case PIP2:
            _registers.PIPCDEP &= ~(c & 0x03);
            _registers.PIPCDEP |= (c & 0x03);
            _bus->sendCommand(LT7680_PIPCDEP);
            _bus->sendData(_registers.PIPCDEP);
            break;
        case CANVAS:
            _registers.AW_COLOR &= ~(c & 0x03);
            _registers.AW_COLOR |= (c & 0x03);
            _bus->sendCommand(LT7680_AW_COLOR);
            _bus->sendData(_registers.AW_COLOR);
            break;
        case BTE_S0:
            _registers.BLT_COLR &= ~((c & 0x03) << 5);
            _registers.BLT_COLR |= ((c & 0x03) << 5);
            _bus->sendCommand(LT7680_BLT_COLR);
            _bus->sendData(_registers.BLT_COLR);
            break;
        case BTE_S1:
            _registers.BLT_COLR &= ~((c & 0x07) << 2);
            _registers.BLT_COLR |= ((c & 0x07) << 2);
            _bus->sendCommand(LT7680_BLT_COLR);
            _bus->sendData(_registers.BLT_COLR);
            break;
        case BTE_DEST:
            _registers.BLT_COLR &= ~(c & 0x03);
            _registers.BLT_COLR |= (c & 0x03);
            _bus->sendCommand(LT7680_BLT_COLR);
            _bus->sendData(_registers.BLT_COLR);
            break;

        case USER_SPACE_1:
            _user_space_config[0].colourDepth = c;
            break;

        case USER_SPACE_2:
            _user_space_config[1].colourDepth = c;
            break;

        case USER_SPACE_3:
            _user_space_config[2].colourDepth = c;
            break;

        case USER_SPACE_4:
            _user_space_config[3].colourDepth = c;
            break;

        default:
            break;
    }
}

void Arduino_LT7680::setImageColourDepth(LT7680_WindowSelection wnd, LT7680_ColourDepth_S1 c)
{
    if(wnd == BTE_S1)
    {
        _registers.BLT_COLR &= ~((c & 0x07) << 2);
        _registers.BLT_COLR |= ((c & 0x07) << 2);
    
        while(isBusy());
        _bus->sendCommand(LT7680_BLT_COLR);
        _bus->sendData(_registers.BLT_COLR);
    }
    else
        setImageColourDepth(wnd, (LT7680_ColourDepth)c);
}

LT7680_ColourDepth Arduino_LT7680::getImageColourDepth(LT7680_WindowSelection wnd)
{
    switch(wnd)
    {
        default:
            
        case MAIN:
            return static_cast<LT7680_ColourDepth>((_registers.MPWCTR >> 2) & 0x03);
        case PIP1:
            return static_cast<LT7680_ColourDepth>((_registers.PIPCDEP >> 2) & 0x03);
        case PIP2:
            return static_cast<LT7680_ColourDepth>(_registers.PIPCDEP & 0x03);
        case CANVAS:
            return static_cast<LT7680_ColourDepth>(_registers.AW_COLOR & 0x03);
        case BTE_S0:
            return static_cast<LT7680_ColourDepth>((_registers.BLT_COLR >> 5) & 0x03);
        case BTE_S1:
            return static_cast<LT7680_ColourDepth>((_registers.BLT_COLR >> 2) & 0x03); // This is strictly not correct, and might need a better way
        case BTE_DEST:
            return static_cast<LT7680_ColourDepth>(_registers.BLT_COLR & 0x03);
        case USER_SPACE_1:
            return _user_space_config[0].colourDepth;
        case USER_SPACE_2:
            return _user_space_config[1].colourDepth;
        case USER_SPACE_3:
            return _user_space_config[2].colourDepth;
        case USER_SPACE_4:
            return _user_space_config[3].colourDepth;
    }
}

void Arduino_LT7680::selectPIPForConfiguration(LT7680_WindowSelection wnd)
{
    SERIAL_PRINTF("selectPIPForConfiguration: %d\n", (uint8_t)wnd);

    if(wnd == PIP1)
    {   
        while(isBusy());
        _registers.MPWCTR &= ~0x10;
        _bus->sendCommand(LT7680_MPWCTR);
        _bus->sendData(_registers.MPWCTR);
    }
    else if(wnd == PIP2)
    {
        while(isBusy());
        _registers.MPWCTR |= 0x10;
        _bus->sendCommand(LT7680_MPWCTR);
        _bus->sendData(_registers.MPWCTR);
    }
}

void Arduino_LT7680::setPIPWindow(LT7680_WindowSelection wnd, uint16_t hw_x, uint16_t hw_y, uint16_t hw_width, uint16_t hw_height)
{
    if(wnd != PIP1 && wnd != PIP2)
        return;

    SERIAL_PRINTF("setPIPWindow: (%d) x:%d, y:%d, w:%d, h:%d\n", (uint8_t)wnd, hw_x, hw_y, hw_width, hw_height);

    selectPIPForConfiguration(wnd);

    hw_x &= 0x1FFC; // Only 13 bits needed and the last two are always 0
    hw_y &= 0x1FFC; // Only 13 bits needed and the last two are always 0
    hw_width &= 0x3FFC; // Only 14 bits needed and the last two are always 0
    hw_height &= 0x3FFC; // Only 14 bits needed and the last two are always 0

    while(isBusy());

    _bus->sendCommand(LT7680_PWIULX_LO);
    _bus->sendData(hw_x & 0xFF);
    _bus->sendCommand(LT7680_PWIULX_HI);
    _bus->sendData((hw_x >> 8) & 0xFF);    

    _bus->sendCommand(LT7680_PWIULY_LO);
    _bus->sendData(hw_y & 0xFF);
    _bus->sendCommand(LT7680_PWIULY_HI);
    _bus->sendData((hw_y >> 8) & 0xFF);    

    _bus->sendCommand(LT7680_PWW_LO);
    _bus->sendData(hw_width & 0xFF);
    _bus->sendCommand(LT7680_PWW_HI);
    _bus->sendData((hw_width >> 8) & 0xFF);    

    _bus->sendCommand(LT7680_PWH_LO);
    _bus->sendData(hw_height & 0xFF);
    _bus->sendCommand(LT7680_PWH_HI);
    _bus->sendData((hw_height >> 8) & 0xFF);    
}

void Arduino_LT7680::setPIPWindowLogical(LT7680_WindowSelection wnd, int16_t x, int16_t y, int16_t width, int16_t height)
{
    int16_t hw_x=0, hw_y=0, hw_w=0, hw_h=0;

    SERIAL_PRINTF("setPIPWindowLogical: (%d) x:%d, y:%d, w:%d, h:%d\n", (uint8_t)wnd, x, y, width, height);

    transformXYHWforRotation(x, y, width, height, &hw_x, &hw_y, &hw_w, &hw_h);
    setPIPWindow(wnd, hw_x, hw_y, hw_w, hw_h);
}

void Arduino_LT7680::enablePIP(LT7680_WindowSelection wnd, bool enable)
{
    SERIAL_PRINTF("enablePIP: (%d) %s\n", (uint8_t)wnd, enable?"true":"false");

    if(wnd != PIP1 && wnd != PIP2)
        return;

    uint8_t val = (wnd==PIP1)?0x80:0x40;

    if(enable)
        _registers.MPWCTR |= val;
    else
        _registers.MPWCTR &= ~val;

    while(isBusy());
    _bus->sendCommand(LT7680_MPWCTR);
    _bus->sendData(_registers.MPWCTR);
}

void Arduino_LT7680::setStartMemoryAddress(LT7680_WindowSelection wnd, uint32_t addr)
{
    switch(wnd)
    {
        case MAIN:
            while(isBusy());
            _mem_address.main_window=addr;
            _bus->sendCommand(LT7680_MISA_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_MISA_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_MISA_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_MISA_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;
        case PIP1:
        case PIP2:
            selectPIPForConfiguration(wnd);
            if(wnd==PIP1)
                _mem_address.pip1_window=addr;
            else
                _mem_address.pip2_window=addr;

            while(isBusy());
            _bus->sendCommand(LT7680_PISA_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_PISA_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_PISA_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_PISA_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;

        case CANVAS:
            _mem_address.canvas=addr;
        
            while(isBusy());
            _bus->sendCommand(LT7680_CVSSA_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_CVSSA_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_CVSSA_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_CVSSA_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;

        case BTE_S0:
            _mem_address.bte_s0=addr;

            while(isBusy());
            _bus->sendCommand(LT7680_S0_STR_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_S0_STR_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_S0_STR_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_S0_STR_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;

        case BTE_S1:
            _mem_address.bte_s1=addr;

            while(isBusy());
            _bus->sendCommand(LT7680_S1_STR_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_S1_STR_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_S1_STR_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_S1_STR_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;

        case BTE_DEST:
            _mem_address.bte_destination=addr;

            while(isBusy());
            _bus->sendCommand(LT7680_DT_STR_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_DT_STR_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_DT_STR_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_DT_STR_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;
        
        case USER_CGRAM:
            _mem_address.user_cgram=addr;
        
            while(isBusy());
            _bus->sendCommand(LT7680_CGRAM_STR0_LO);
            _bus->sendData(addr & 0xFF);
            _bus->sendCommand(LT7680_CGRAM_STR0_MIDLO);
            _bus->sendData((addr >> 8) & 0xFF);
            _bus->sendCommand(LT7680_CGRAM_STR0_MIDHI);
            _bus->sendData((addr >> 16) & 0xFF);
            _bus->sendCommand(LT7680_CGRAM_STR0_HI);
            _bus->sendData((addr >> 24) & 0xFF);
            break;

        case USER_SPACE_1:
            _mem_address.user_space_1 = addr;
            break;

        case USER_SPACE_2:
            _mem_address.user_space_2 = addr;
            break;

        case USER_SPACE_3:
            _mem_address.user_space_3 = addr;
            break;

        case USER_SPACE_4:
            _mem_address.user_space_4 = addr;
            break;

    }
}

uint32_t Arduino_LT7680::getStartMemoryAddress(LT7680_WindowSelection wnd)
{
    switch(wnd)
    {
        default:
        case MAIN:
            return _mem_address.main_window;
        case PIP1:
            return _mem_address.pip1_window;
        case PIP2:
            return _mem_address.pip2_window;
        case CANVAS:
            return _mem_address.canvas;
        case BTE_S0:
            return _mem_address.bte_s0;
        case BTE_S1:
            return _mem_address.bte_s1;
        case BTE_DEST:
            return _mem_address.bte_destination;
        case USER_CGRAM:
            return _mem_address.user_cgram; 
        case USER_SPACE_1:
            return _mem_address.user_space_1;
        case USER_SPACE_2:
            return _mem_address.user_space_2;
        case USER_SPACE_3:
            return _mem_address.user_space_3;
        case USER_SPACE_4:
            return _mem_address.user_space_4;
    }
}

void Arduino_LT7680::setWindowWidth(LT7680_WindowSelection wnd, uint16_t hw_width)
{
    SERIAL_PRINTF("setWindowWidth: (%d) %d\n", (uint8_t)wnd, hw_width);

    switch(wnd)
    {
        case MAIN:
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_MIW_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_MIW_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;

        case PIP1:
        case PIP2:
            selectPIPForConfiguration(wnd);
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_PIW_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_PIW_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;
        
        case CANVAS:
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_CVS_IMWTH_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_CVS_IMWTH_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;

        case BTE_S0:
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_S0_WTH_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_S0_WTH_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;

        case BTE_S1:
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_S1_WTH_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_S1_WTH_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;

        case BTE_DEST:
            // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            hw_width &= 0x1FFC;

            while(isBusy());
            _bus->sendCommand(LT7680_DT_WTH_LO);
            _bus->sendData(hw_width & 0xFF);
            _bus->sendCommand(LT7680_DT_WTH_HI);
            _bus->sendData((hw_width >> 8) & 0xFF);
            break;

        default:
            break;
    }
}

void Arduino_LT7680::setWindowUpperLeftXY(LT7680_WindowSelection wnd, uint16_t hw_x, uint16_t hw_y)
{
    // Maximum of 13 bits allowed
    hw_y &= 0x1FFF;

    SERIAL_PRINTF("setWindowUpperLeftXY: (%d) %d, %d\n", (uint8_t)wnd, hw_x, hw_y);

    switch(wnd)
    {
        case MAIN:
            hw_x &= 0x1FFC; // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            while(isBusy());
            _bus->sendCommand(LT7680_MWULX_LO);
            _bus->sendData(hw_x & 0xFF);
            _bus->sendCommand(LT7680_MWULX_HI);
            _bus->sendData((hw_x >> 8) & 0xFF);
            _bus->sendCommand(LT7680_MWULY_LO);
            _bus->sendData(hw_y & 0xFF);
            _bus->sendCommand(LT7680_MWULY_HI);
            _bus->sendData((hw_y >> 8) & 0xFF);
            break;

        case PIP1:
        case PIP2:
            hw_x &= 0x1FFC; // The two least significant bits are always 0, and a maximum of 13 bits are allowed
            selectPIPForConfiguration(wnd);
            
            while(isBusy());
            _bus->sendCommand(LT7680_PWDULX_LO);
            _bus->sendData(hw_x & 0xFF);
            _bus->sendCommand(LT7680_PWDULX_HI);
            _bus->sendData((hw_x >> 8) & 0xFF);
            _bus->sendCommand(LT7680_PWDULY_LO);
            _bus->sendData(hw_y & 0xFF);
            _bus->sendCommand(LT7680_PWDULY_HI);
            _bus->sendData((hw_y >> 8) & 0xFF);
            break;
        
        case BTE_S0:
            hw_x &= 0x1FFF; // Maximum of 13 bits are allowed
            while(isBusy());
            _bus->sendCommand(LT7680_S0_X_LO);
            _bus->sendData(hw_x & 0xFF);
            _bus->sendCommand(LT7680_S0_X_HI);
            _bus->sendData((hw_x >> 8) & 0xFF);
            _bus->sendCommand(LT7680_S0_Y_LO);
            _bus->sendData(hw_y & 0xFF);
            _bus->sendCommand(LT7680_S0_Y_HI);
            _bus->sendData((hw_y >> 8) & 0xFF);
            break;

        case BTE_S1:
            hw_x &= 0x1FFF; // Maximum of 13 bits are allowed
            while(isBusy());
            _bus->sendCommand(LT7680_S1_X_LO);
            _bus->sendData(hw_x & 0xFF);
            _bus->sendCommand(LT7680_S1_X_HI);
            _bus->sendData((hw_x >> 8) & 0xFF);
            _bus->sendCommand(LT7680_S1_Y_LO);
            _bus->sendData(hw_y & 0xFF);
            _bus->sendCommand(LT7680_S1_Y_HI);
            _bus->sendData((hw_y >> 8) & 0xFF);
            break;

        case BTE_DEST:
            hw_x &= 0x1FFF; // Maximum of 13 bits are allowed
            while(isBusy());
            _bus->sendCommand(LT7680_DT_X_LO);
            _bus->sendData(hw_x & 0xFF);
            _bus->sendCommand(LT7680_DT_X_HI);
            _bus->sendData((hw_x >> 8) & 0xFF);
            _bus->sendCommand(LT7680_DT_Y_LO);
            _bus->sendData(hw_y & 0xFF);
            _bus->sendCommand(LT7680_DT_Y_HI);
            _bus->sendData((hw_y >> 8) & 0xFF);
            break;

        default:
            break;
    }
}

void Arduino_LT7680::setWindowUpperLeftXYLogical(LT7680_WindowSelection wnd, int16_t x, int16_t y, int16_t w, int16_t h)
{
    SERIAL_PRINTF("setWindowUpperLeftXYLogical: (%d) %d, %d\n", (uint8_t)wnd, x, y);

    int16_t hw_x=0, hw_y=0, hw_w=0, hw_h=0;
    transformXYHWforRotation(x, y, w, h, &hw_x, &hw_y, &hw_w, &hw_h);
    setWindowUpperLeftXY(wnd, hw_x, hw_y);
}

void Arduino_LT7680::setActiveWindowArea(uint16_t hw_x, uint16_t hw_y, uint16_t hw_width, uint16_t hw_height)
{
    SERIAL_PRINTF("setActiveWindowArea: x:%d, y:%d, w:%d, h:%d\n", hw_x, hw_y, hw_width, hw_height);

    // Limited to 13 bits
    hw_x &= 0x1FFF;
    hw_y &= 0x1FFF;

    // Limited to 14 bits
    hw_width &= 0x3FFF;
    hw_height &= 0x3FFF;

    while(isBusy());

    // Set x coordinate
    _bus->sendCommand(LT7680_AWUL_X_LO);
    _bus->sendData(hw_x & 0xFF);
    _bus->sendCommand(LT7680_AWUL_X_HI);
    _bus->sendData((hw_x >> 8) & 0xFF);

    // Set y coordinate
    _bus->sendCommand(LT7680_AWUL_Y_LO);
    _bus->sendData(hw_y & 0xFF);
    _bus->sendCommand(LT7680_AWUL_Y_HI);
    _bus->sendData((hw_y >> 8) & 0xFF);

    // Set width
    _bus->sendCommand(LT7680_AW_WTH_LO);
    _bus->sendData(hw_width & 0xFF);
    _bus->sendCommand(LT7680_AW_WTH_HI);
    _bus->sendData((hw_width >> 8) & 0xFF);

    // Set height
    _bus->sendCommand(LT7680_AW_HT_LO);
    _bus->sendData(hw_height & 0xFF);
    _bus->sendCommand(LT7680_AW_HT_HI);
    _bus->sendData((hw_height >> 8) & 0xFF);
}

void Arduino_LT7680::setCanvasTo(LT7680_WindowSelection wnd)
{
    switch(wnd)
    {
        case CANVAS:
            return;
        case MAIN:
            setStartMemoryAddress(CANVAS, _mem_address.main_window);
            break;

        case PIP1:
            setStartMemoryAddress(CANVAS, _mem_address.pip1_window);
            break;

        case PIP2:
            setStartMemoryAddress(CANVAS, _mem_address.pip2_window);
            break;

        case BTE_S0:
            setStartMemoryAddress(CANVAS, _mem_address.bte_s0);
            break;

        case BTE_S1:
            setStartMemoryAddress(CANVAS, _mem_address.bte_s1);
            break;

        case BTE_DEST:
            setStartMemoryAddress(CANVAS, _mem_address.bte_destination);
            break;

        case USER_CGRAM:
            setStartMemoryAddress(CANVAS, _mem_address.user_cgram);
            break;

        case USER_SPACE_1:
            setStartMemoryAddress(CANVAS, _mem_address.user_space_1);
            break;

        case USER_SPACE_2:
            setStartMemoryAddress(CANVAS, _mem_address.user_space_2);
            break;

        case USER_SPACE_3:
            setStartMemoryAddress(CANVAS, _mem_address.user_space_3);
            break;

        case USER_SPACE_4:
            setStartMemoryAddress(CANVAS, _mem_address.user_space_4);
            break;
    }

    setImageColourDepth(CANVAS, getImageColourDepth(wnd));
}

void Arduino_LT7680::setMainWindowTo(LT7680_WindowSelection wnd)
{
    switch(wnd)
    {
        case CANVAS:
            setStartMemoryAddress(MAIN, _mem_address.canvas);
            break;
        case MAIN:
            break;
        case PIP1:
            setStartMemoryAddress(MAIN, _mem_address.pip1_window);
            break;

        case PIP2:
            setStartMemoryAddress(MAIN, _mem_address.pip2_window);
            break;

        case BTE_S0:
            setStartMemoryAddress(MAIN, _mem_address.bte_s0);
            break;

        case BTE_S1:
            setStartMemoryAddress(MAIN, _mem_address.bte_s1);
            break;

        case BTE_DEST:
            setStartMemoryAddress(MAIN, _mem_address.bte_destination);
            break;

        case USER_CGRAM:
            setStartMemoryAddress(MAIN, _mem_address.user_cgram);
            break;

        case USER_SPACE_1:
            setStartMemoryAddress(MAIN, _mem_address.user_space_1);
            break;

        case USER_SPACE_2:
            setStartMemoryAddress(MAIN, _mem_address.user_space_2);
            break;

        case USER_SPACE_3:
            setStartMemoryAddress(MAIN, _mem_address.user_space_3);
            break;

        case USER_SPACE_4:
            setStartMemoryAddress(MAIN, _mem_address.user_space_4);
            break;
    }
    setImageColourDepth(MAIN, getImageColourDepth(wnd));
}

void Arduino_LT7680::swapCanvasWithMainWindow()
{
    uint32_t old_mw = _mem_address.main_window;
    LT7680_ColourDepth c = getImageColourDepth(MAIN);

    setStartMemoryAddress(MAIN, _mem_address.canvas);
    setImageColourDepth(MAIN, getImageColourDepth(CANVAS));
    setStartMemoryAddress(CANVAS, old_mw);
    setImageColourDepth(CANVAS,c);
}

size_t Arduino_LT7680::getImageMemorySize(LT7680_WindowSelection wnd)
{
    switch(getImageColourDepth(wnd))
    {
        case DEPTH_8BPP:
            return _config.horizontal_width * _config.vertical_height;

        case DEPTH_16BPP:
            return _config.horizontal_width * _config.vertical_height * 2;

        default:
        case DEPTH_24BPP:
            return _config.horizontal_width * _config.vertical_height * 3;
    }
}

uint8_t Arduino_LT7680::reverseBits(uint8_t b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

uint16_t Arduino_LT7680::rotate1bitBitmapByLine(uint8_t *input, uint16_t org_w, uint16_t org_h, uint16_t y, uint8_t *output)
{
    int16_t org_bytesPerRow = (org_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint16_t new_bytesPerRow = (org_h + 7) / 8;
    uint16_t byteOfInterest = (y / 8);
    uint8_t bitOfInterest = 7 - (y % 8);
            
    switch(_lt7680_rotation)
    {
        case ROTATE_NORMAL:
            memcpy(output, &input[org_bytesPerRow * y], org_bytesPerRow);
            return org_bytesPerRow;

        case ROTATE_RIGHT_90:
            bitOfInterest = 1 << bitOfInterest;

//            Serial.printf("BitofInterest: %02x, ByteOfInterest: %02x\n",bitOfInterest,byteOfInterest);

            for(uint16_t i=0; i< org_h; i++)
            {
                if(i % 8 == 0)
                    output[i / 8] = 0;

                output[i / 8] |=
                    (input[org_bytesPerRow * i + byteOfInterest] & bitOfInterest)
                    ?
                        (1 << (7 - (i % 8)))
                    :
                        0;
            }

            return new_bytesPerRow;

        case ROTATE_LEFT_90:
            byteOfInterest = (org_w - y - 1) / 8;
            bitOfInterest = 8 - ((org_w - y) % 8);
            bitOfInterest = (1 << (bitOfInterest<8?bitOfInterest:0));

            //Serial.printf("BitofInterest: %02x, ByteOfInterest: %02x\n",bitOfInterest,byteOfInterest);

            for(uint16_t i=0; i< org_h; i++)
            {
                if(i % 8 == 0)
                    output[i / 8] = 0;

                output[i / 8] |=
                    (input[org_bytesPerRow * (org_h - i -1) + byteOfInterest] & bitOfInterest)
                    ?
                        (1 << (7 - (i % 8)))
                    :
                        0;
            }
            
            return new_bytesPerRow;

        case ROTATE_180:

            uint8_t lastBit = org_w % 8;

            for(uint8_t i=0; i<org_bytesPerRow; i++)
            {
                output[i] = reverseBits(input[(org_bytesPerRow * y) + (org_bytesPerRow - i) - 1]) << (lastBit?(8 - lastBit):0);
                if(i<org_bytesPerRow - 1)
                    output[i] |= (reverseBits(input[(org_bytesPerRow * y) + (org_bytesPerRow - i) - 2]) >> (lastBit?lastBit:8)); 
            }

            return org_bytesPerRow;
    }
    return 0;
}

uint16_t Arduino_LT7680::rotate1bitBitmapByLine(const uint8_t input[], uint16_t org_w, uint16_t org_h, uint16_t y, uint8_t *output)
{
    int16_t org_bytesPerRow = (org_w + 7) / 8; // Bitmap scanline pad = whole byte
    uint16_t new_bytesPerRow = (org_h + 7) / 8;
    uint16_t byteOfInterest = (y / 8);
    uint8_t bitOfInterest = 7 - (y % 8);
    
    switch(_lt7680_rotation)
    {
        case ROTATE_NORMAL:
            memcpy(output, &input[org_bytesPerRow * y], org_bytesPerRow);
            return org_bytesPerRow;

        case ROTATE_RIGHT_90:
            bitOfInterest = 1 << bitOfInterest;

//            Serial.printf("BitofInterest: %02x, ByteOfInterest: %02x\n",bitOfInterest,byteOfInterest);

            for(uint16_t i=0; i< org_h; i++)
            {
                if(i % 8 == 0)
                    output[i / 8] = 0;

                output[i / 8] |=
                    (pgm_read_byte(&input[org_bytesPerRow * i + byteOfInterest]) & bitOfInterest)
                    ?
                        (1 << (7 - (i % 8)))
                    :
                        0;
            }

            return new_bytesPerRow;

        case ROTATE_LEFT_90:
            byteOfInterest = (org_w - y - 1) / 8;
            bitOfInterest = 8 - ((org_w - y) % 8);
            bitOfInterest = (1 << (bitOfInterest<8?bitOfInterest:0));

            //Serial.printf("BitofInterest: %02x, ByteOfInterest: %02x\n",bitOfInterest,byteOfInterest);

            for(uint16_t i=0; i< org_h; i++)
            {
                if(i % 8 == 0)
                    output[i / 8] = 0;

                output[i / 8] |=
                    (pgm_read_byte(&input[org_bytesPerRow * (org_h - i -1) + byteOfInterest]) & bitOfInterest)
                    ?
                        (1 << (7 - (i % 8)))
                    :
                        0;
            }
            
            return new_bytesPerRow;

        case ROTATE_180:

            uint8_t lastBit = org_w % 8;

            for(uint8_t i=0; i<org_bytesPerRow; i++)
            {
                output[i] = reverseBits(pgm_read_byte(&input[(org_bytesPerRow * y) + (org_bytesPerRow - i) - 1])) << (lastBit?(8 - lastBit):0);
                if(i<org_bytesPerRow - 1)
                    output[i] |= (reverseBits(pgm_read_byte(&input[(org_bytesPerRow * y) + (org_bytesPerRow - i) - 2])) >> (lastBit?lastBit:8)); 
            }

            return org_bytesPerRow;
    }
    return 0;
}

void Arduino_LT7680::transformXYforRotation(int16_t logical_x, int16_t logical_y, int16_t *hardware_x, int16_t *hardware_y)
{
    switch(_lt7680_rotation)
    {
        case ROTATE_NORMAL:
            *hardware_x = logical_x;
            *hardware_y = logical_y;
            break;

        case ROTATE_180:
            *hardware_x = _config.horizontal_width - logical_x;// - 1;
            *hardware_y = _config.vertical_height - logical_y;// - 1;
            break;

        case ROTATE_LEFT_90:
            *hardware_x = logical_y;
            *hardware_y = _config.vertical_height - logical_x;// - 1;
            break;

        case ROTATE_RIGHT_90:
            *hardware_x = _config.horizontal_width - logical_y;// - 1;
            *hardware_y = logical_x;
    }

    //Serial.printf("l_x: %d, l_y: %d, h_x: %d, h_y: %d\n", logical_x, logical_y, *hardware_x, *hardware_y);
}

void Arduino_LT7680::transformXYHWforRotation(int16_t logical_x, int16_t logical_y, int16_t logical_width, int16_t logical_height, int16_t *hardware_x, int16_t *hardware_y, int16_t *hardware_width, int16_t *hardware_height)
{
    transformXYforRotation(logical_x, logical_y, hardware_x, hardware_y);

    switch(_lt7680_rotation)
    {
        case ROTATE_180:
            *hardware_x -= logical_width;
            *hardware_y -= logical_height;
            break;
        case ROTATE_NORMAL:
            *hardware_width = logical_width;
            *hardware_height = logical_height;
            break;

        case ROTATE_LEFT_90:
            *hardware_y -= logical_width;
            *hardware_width = logical_height;
            *hardware_height = logical_width;
            break;

        case ROTATE_RIGHT_90:
            *hardware_x -= logical_height;
            *hardware_width = logical_height;
            *hardware_height = logical_width;
            break;
    }
}


void Arduino_LT7680::setGraphicPenPos(uint16_t x, uint16_t y)
{
    int16_t hw_x, hw_y;
    transformXYforRotation(x, y, &hw_x, &hw_y);

    // Only 13 bits are used
    _registers16.CURH = (hw_x & 0x1FFF);
    _registers16.CURV = (hw_y & 0x1FFF);

    while(isBusy());

    // Set the graphic pen to the location
    _bus->sendCommand(LT7680_CURH_LO);
    _bus->sendData(_registers16.CURH & 0xFF);
    _bus->sendCommand(LT7680_CURH_HI);
    _bus->sendData((_registers16.CURH >> 8) & 0xFF);
    _bus->sendCommand(LT7680_CURV_LO);
    _bus->sendData(_registers16.CURV & 0xFF);
    _bus->sendCommand(LT7680_CURV_HI);
    _bus->sendData((_registers16.CURV >> 8) & 0xFF);
}

void Arduino_LT7680::resetActiveWindowToWholeScreen()
{
    setActiveWindowArea(0, 0, _config.horizontal_width, _config.vertical_height);
}

void Arduino_LT7680::setStraightShapeCoordinates(uint16_t x0, uint16_t y0)
{
    // Only 13 bits are allowed
    x0 &= 0x1FFF;
    y0 &= 0x1FFF;

    while(isBusy());

    _bus->sendCommand(LT7680_DLHSR_LO);
    _bus->sendData(x0 & 0xFF);
    _bus->sendCommand(LT7680_DLHSR_HI);
    _bus->sendData((x0 >> 8) & 0xFF);

    _bus->sendCommand(LT7680_DLVSR_LO);
    _bus->sendData(y0 & 0xFF);
    _bus->sendCommand(LT7680_DLVSR_HI);
    _bus->sendData((y0 >> 8) & 0xFF);
}

void Arduino_LT7680::setStraightShapeCoordinates(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    setStraightShapeCoordinates(x0, y0);

    // Only 13 bits are allowed
    x1 &= 0x1FFF;
    y1 &= 0x1FFF;

    while(isBusy());

    _bus->sendCommand(LT7680_DLHER_LO);
    _bus->sendData(x1 & 0xFF);
    _bus->sendCommand(LT7680_DLHER_HI);
    _bus->sendData((x1 >> 8) & 0xFF);

    _bus->sendCommand(LT7680_DLVER_LO);
    _bus->sendData(y1 & 0xFF);
    _bus->sendCommand(LT7680_DLVER_HI);
    _bus->sendData((y1 >> 8) & 0xFF);
}

void Arduino_LT7680::setStraightShapeCoordinates(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    setStraightShapeCoordinates(x0, y0, x1, y1);

    // Only 13 bits are allowed
    x2 &= 0x1FFF;
    y2 &= 0x1FFF;

    while(isBusy());

    _bus->sendCommand(LT7680_DTPH_LO);
    _bus->sendData(x2 & 0xFF);
    _bus->sendCommand(LT7680_DTPH_HI);
    _bus->sendData((x2 >> 8) & 0xFF);

    _bus->sendCommand(LT7680_DTPV_LO);
    _bus->sendData(y2 & 0xFF);
    _bus->sendCommand(LT7680_DTPV_HI);
    _bus->sendData((y2 >> 8) & 0xFF);
}

void Arduino_LT7680::setCurvedShapeParameters(uint16_t major_radius, uint16_t minor_radius)
{
    // Only 13 bits are allowed
    major_radius &= 0x1FFF;
    minor_radius &= 0x1FFF;

    while(isBusy());

    _bus->sendCommand(LT7680_ELL_A_LO);
    _bus->sendData(major_radius & 0xFF);
    _bus->sendCommand(LT7680_ELL_A_HI);
    _bus->sendData((major_radius >> 8) & 0xFF);
    _bus->sendCommand(LT7680_ELL_B_LO);
    _bus->sendData(minor_radius & 0xFF);
    _bus->sendCommand(LT7680_ELL_B_HI);
    _bus->sendData((minor_radius >> 8) & 0xFF);

}

void Arduino_LT7680::setCurvedShapeParameters(uint16_t major_radius, uint16_t minor_radius, uint16_t centre_x, uint16_t centre_y)
{
    setCurvedShapeParameters(major_radius, minor_radius);

    // Only 13 bits are allowed
    centre_x &= 0x1FFF;
    centre_y &= 0x1FFF;


    while(isBusy());

    _bus->sendCommand(LT7680_DEHR_LO);
    _bus->sendData(centre_x & 0xFF);
    _bus->sendCommand(LT7680_DEHR_HI);
    _bus->sendData((centre_x >> 8) & 0xFF);

    _bus->sendCommand(LT7680_DEVR_LO);
    _bus->sendData(centre_y & 0xFF);
    _bus->sendCommand(LT7680_DEVR_HI);
    _bus->sendData((centre_y >> 8) & 0xFF);
}

void Arduino_LT7680::startDrawingShape(LT7680_Draw_Shape_Option shp, bool Fill, bool PolylineClose)
{
    uint8_t data=0;

    switch(shp)
    {
        case DRAW_LINE:
        case DRAW_TRIANGLE:
        case DRAW_QUAD:
        case DRAW_PENT:
        case DRAW_POLY_3EP:
        case DRAW_POLY_4EP:
        case DRAW_POLY_5EP:
            while(isBusy());
            _bus->sendCommand(LT7680_DCR0);
            _bus->sendData(0x80 | (Fill?0x20:0) | (PolylineClose?1:0) | ((shp & 0x0F) << 1));
            return;

        case DRAW_RECT:
            data = 0x20;
            break;
        case DRAW_ELLIPSE:
            data = 0x00;
            break;
        case DRAW_RND_RECT:
            data = 0x30;
            break;
        case DRAW_ARC_UPRT:
            data = 0x12;
            break;
        case DRAW_ARC_UPLT:
            data = 0x11;
            break;
        case DRAW_ARC_LOLT:
            data = 0x10;
            break;
        case DRAW_ARC_LORT:
            data = 0x13;
            break;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_DCR1);
    _bus->sendData(0x80 | data | (Fill?0x40:0));
}

void Arduino_LT7680::setForegroundColour(uint8_t red, uint8_t green, uint8_t blue)
{
    switch(getImageColourDepth(CANVAS))
    {
        case DEPTH_8BPP:
            red = (red & 0x07) << 5;
            green = (green & 0x07) << 5;
            blue = (blue & 0x03) << 6;
            break;
        case DEPTH_16BPP:
            red = (red & 0x1F) << 3;
            green = (green & 0x3F) << 2;
            blue = (blue & 0x1F) << 3;
            break;
        default:
            break;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_FGCR);
    _bus->sendData(red);
    _bus->sendCommand(LT7680_FGCG);
    _bus->sendData(green);
    _bus->sendCommand(LT7680_FGCB);
    _bus->sendData(blue);
}

void Arduino_LT7680::setForegroundColour(uint16_t colour)
{
    if(getImageColourDepth(CANVAS)==LT7680_ColourDepth::DEPTH_8BPP)
    {
        while(isBusy());
        _bus->sendCommand(LT7680_FGCR);
        _bus->sendData((colour & 0xE000) >> 8);
        _bus->sendCommand(LT7680_FGCG);
        _bus->sendData((colour & 0x0300) >> 3);
        _bus->sendCommand(LT7680_FGCB);
        _bus->sendData((colour & 0x0018) << 3);
        return;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_FGCR);
    _bus->sendData((colour & 0xF800) >> 8);
    _bus->sendCommand(LT7680_FGCG);
    _bus->sendData((colour & 0x07E0) >> 3);
    _bus->sendCommand(LT7680_FGCB);
    _bus->sendData((colour & 0x001F) << 3);
}

void Arduino_LT7680::setBackgroundColour(uint8_t red, uint8_t green, uint8_t blue)
{
    switch(getImageColourDepth(CANVAS))
    {
        case DEPTH_8BPP:
            red = (red & 0x07) << 5;
            green = (green & 0x07) << 5;
            blue = (blue & 0x03) << 6;
            break;
        case DEPTH_16BPP:
            red = (red & 0x1F) << 3;
            green = (green & 0x3F) << 2;
            blue = (blue & 0x1F) << 3;
            break;
        default:
            break;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_BGCR);
    _bus->sendData(red);
    _bus->sendCommand(LT7680_BGCG);
    _bus->sendData(green);
    _bus->sendCommand(LT7680_BGCB);
    _bus->sendData(blue);
}

void Arduino_LT7680::setBackgroundColour(uint16_t colour)
{
    if(getImageColourDepth(CANVAS)==LT7680_ColourDepth::DEPTH_8BPP)
    {
        while(isBusy());
        _bus->sendCommand(LT7680_BGCR);
        _bus->sendData((colour & 0xE000) >> 8);
        _bus->sendCommand(LT7680_BGCG);
        _bus->sendData((colour & 0x0300) >> 3);
        _bus->sendCommand(LT7680_BGCB);
        _bus->sendData((colour & 0x0018) << 3);
        return;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_BGCR);
    _bus->sendData((colour & 0xF800) >> 8);
    _bus->sendCommand(LT7680_BGCG);
    _bus->sendData((colour & 0x07E0) >> 3);
    _bus->sendCommand(LT7680_BGCB);
    _bus->sendData((colour & 0x001F) << 3);
}

void Arduino_LT7680::writeColor(uint32_t color, LT7680_ColourDepth destinationCD)
{
    switch(destinationCD)
    {
        case DEPTH_8BPP:
            _bus->sendData(((color & 0x00E00000) >> 16) | ((color & 0x0000E000) >> 11) | ((color & 0x000000C0) >> 6));
            break;
        case DEPTH_16BPP:
            _bus->sendData(((color & 0x00F80000) >> 16) | ((color & 0x0000E000) >> 13));
            _bus->sendData(((color & 0x00001E00) >> 8) | ((color & 0x000000F8) >> 3 ));
            break;
        case DEPTH_24BPP:
            _bus->sendData(color & 0x000000FF);
            _bus->sendData((color & 0x0000FF00) >> 8);
            _bus->sendData((color & 0x00FF0000) >> 16);
    }
}

void Arduino_LT7680::writeColor(uint16_t color, LT7680_ColourDepth destinationCD)
{
    switch(destinationCD)
    {
        case DEPTH_8BPP:
            _bus->sendData(((color & 0xE000) >> 8) | ((color & 0x0300) >> 6) | ((color & 0x018) >> 3));
            break;
        case DEPTH_16BPP:
            _bus->sendData(color & 0xFF);
            _bus->sendData((color >> 8) & 0xFF);
            break;
        case DEPTH_24BPP:
            _bus->sendData(((color & 0x001F) << 3) | ((color & 0x001F) >> 2));
            _bus->sendData(((color & 0x07E0) >> 3) | ((color & 0x07E0) >> 9));
            _bus->sendData(((color & 0xF800) >> 8) | ((color & 0xF800) >> 13));
    }
}

void Arduino_LT7680::writeColor(uint8_t color, LT7680_ColourDepth destinationCD)
{
    switch(destinationCD)
    {
        case DEPTH_8BPP:
            _bus->sendData(color);
            break;
        case DEPTH_16BPP:
            _bus->sendData((color & 0xE0) | ((color & 0x1C) >> 2));
            _bus->sendData((color & 0x03) << 3);
            break;
        case DEPTH_24BPP:
            _bus->sendData((color & 0xE0));
            _bus->sendData((color & 0x1C) << 3);
            _bus->sendData((color & 0x03) << 6);
    }
}

void Arduino_LT7680::BTE_enable()
{
    _registers.BLT_CTRL0 |= 0x10;

    while(isBusy());
    _bus->sendCommand(LT7680_BLT_CTRL0);
    _bus->sendData(_registers.BLT_CTRL0);
}

void Arduino_LT7680::BTE_setWindowSize(uint16_t width, uint16_t height)
{
    width &= 0x1FFC; // Maximum of 13 bits are allowed
    height &= 0x1FFF; // Maximum of 13 bits are allowed

    while(isBusy());
    _bus->sendCommand(LT7680_BLT_WTH_LO);
    _bus->sendData(width & 0xFF);
    _bus->sendCommand(LT7680_BLT_WTH_HI);
    _bus->sendData((width >> 8) & 0xFF);
    _bus->sendCommand(LT7680_BLT_HIG_LO);
    _bus->sendData(height & 0xFF);
    _bus->sendCommand(LT7680_BLT_HIG_HI);
    _bus->sendData((height >> 8) & 0xFF);

}

void Arduino_LT7680::BTE_setAlphaBlending(uint8_t alpha)
{
    alpha &= 0x2F;

    while(isBusy());
    _bus->sendCommand(LT7680_APB_CTRL);
    _bus->sendData(alpha);
}

void Arduino_LT7680::BTE_setOperation(LT7680_BTE_Operation_Code op, uint8_t ROP)
{
    while(isBusy());
    _bus->sendCommand(LT7680_BLT_CTRL1);
    _bus->sendData((op & 0xF) | ((ROP & 0xF) << 4));
}


void Arduino_LT7680::setLT7680Rotation(LT7680_Rotation r)
{
    _lt7680_rotation = r;

    switch (r)
    {
        case ROTATE_RIGHT_90:
            _registers.MACR &= ~0x02;
            _registers.MACR |= 0x04;
            break;
        case ROTATE_180:
            _registers.MACR &= ~0x04;
            _registers.MACR |= 0x02;
            break;
        case ROTATE_LEFT_90:
            _registers.MACR |= 0x02;
            _registers.MACR |= 0x04;
            break;
        default: 
            _registers.MACR &= ~0x06;
            break;
    }

    while(isBusy());
    _bus->sendCommand(LT7680_MACR);
    _bus->sendData(_registers.MACR);
}


void Arduino_LT7680::setCharacterSource(LT7680_CharacterGeneratorSource cgrom)
{
    uint8_t val = (uint8_t)cgrom & 3;
    
    if(val==3)
        return;

    val <<= 6;
    _registers.CCR0 &= ~val;
    _registers.CCR0 |= val; 

    while(isBusy());
    _bus->sendCommand(LT7680_CCR0);
    _bus->sendData(_registers.CCR0);
}


void Arduino_LT7680::setCharacterHeight(LT7680_CharacterHeight height)
{
    uint8_t val = (uint8_t)height & 3;
    
    if(val==3)
        return;

    val <<= 4;
    _registers.CCR0 &= ~val;
    _registers.CCR0 |= val; 

    while(isBusy());
    _bus->sendCommand(LT7680_CCR0);
    _bus->sendData(_registers.CCR0);
}

void Arduino_LT7680::setCharacterInternalCGROM(LT7680_CharacterSelection sel)
{
    uint8_t val = (uint8_t)sel & 3;

    _registers.CCR0 &= ~val;
    _registers.CCR0 |= val; 

    while(isBusy());
    _bus->sendCommand(LT7680_CCR0);
    _bus->sendData(_registers.CCR0);
}

void Arduino_LT7680::setCharacterFullAlignment(bool enable)
{
    uint8_t val = enable?0x80:0;

    _registers.CCR1 &= ~val;
    _registers.CCR1 |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_CCR1);
    _bus->sendData(_registers.CCR1);
}

void Arduino_LT7680::setCharacterChromaKeying(bool enable)
{
    uint8_t val = enable?0x40:0;

    _registers.CCR1 &= ~val;
    _registers.CCR1 |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_CCR1);
    _bus->sendData(_registers.CCR1);
}

void Arduino_LT7680::setCharacterRotation(bool rotateAndFlip)
{
    uint8_t val = rotateAndFlip?0x10:0;

    _registers.CCR1 &= ~val;
    _registers.CCR1 |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_CCR1);
    _bus->sendData(_registers.CCR1);
}

void Arduino_LT7680::setCharacterWidthEnlargement(uint8_t zoomFactor)
{
    if(zoomFactor == 0)
        zoomFactor = 1;
    else if(zoomFactor > 4)
        zoomFactor = 4;
    
    zoomFactor = (zoomFactor - 1) << 2;
    _registers.CCR1 &= ~zoomFactor;
    _registers.CCR1 |= zoomFactor;

    while(isBusy());
    _bus->sendCommand(LT7680_CCR1);
    _bus->sendData(_registers.CCR1);
}

void Arduino_LT7680::setCharacterHeightEnlargement(uint8_t zoomFactor)
{
    if(zoomFactor == 0)
        zoomFactor = 1;
    else if(zoomFactor > 4)
        zoomFactor = 4;
    
    zoomFactor--;
    _registers.CCR1 &= ~zoomFactor;
    _registers.CCR1 |= zoomFactor;

    while(isBusy());
    _bus->sendCommand(LT7680_CCR1);
    _bus->sendData(_registers.CCR1);
}

void Arduino_LT7680::setCharacterLineGap(uint8_t pixelGap)
{
    pixelGap &= 0x1F;

    while(isBusy());
    _bus->sendCommand(LT7680_FLDR);
    _bus->sendData(pixelGap);
}

void Arduino_LT7680::setCharacterToCharacterGap(uint8_t pixelGap)
{
    pixelGap &= 0x3F;

    while(isBusy());
    _bus->sendCommand(LT7680_F2FSSR);
    _bus->sendData(pixelGap);
}

void Arduino_LT7680::enterTextMode()
{
    setCharacterSource(CGROM_INTERNAL);
    setCharacterHeight(CHAR_HEIGHT_DOTS_16);
    setCharacterInternalCGROM(CHAR_ISO_IEC_8859_1);
    setCharacterFullAlignment(true);
    setCharacterChromaKeying(false);
    setCharacterRotation(true);
    setCharacterHeightEnlargement(1);
    setCharacterWidthEnlargement(1);
    setCharacterLineGap(1);
    setCharacterToCharacterGap(1);
    setTextMode();
}

void Arduino_LT7680::setTextMode()
{
    _registers.ICR |= 0x04;

    while(isBusy());
    _bus->sendCommand(LT7680_ICR);
    _bus->sendData(_registers.ICR);
}

void Arduino_LT7680::setTextCursorPosition(uint16_t x, uint16_t y)
{
    // Only 13 bits are allowed
    x &= 0x1FFF;
    y &= 0x1FFF;

    while(isBusy());
    _bus->sendCommand(LT7680_F_CURX_LO);
    _bus->sendData(x & 0xFF);
    _bus->sendCommand(LT7680_F_CURX_HI);
    _bus->sendData((x >> 8) & 0xFF);

    _bus->sendCommand(LT7680_F_CURY_LO);
    _bus->sendData(y & 0xFF);
    _bus->sendCommand(LT7680_F_CURY_HI);
    _bus->sendData((y >> 8) & 0xFF);    
}

void Arduino_LT7680::writeText(char c)
{
    while(isBusy());
    _bus->sendCommand(LT7680_MRWDP);
    _bus->sendData(c);
}

void Arduino_LT7680::writeText(const char *chars)
{
    while(isBusy());
    _bus->sendCommand(LT7680_MRWDP);
    uint16_t len = strlen(chars);
    for(uint16_t i=0; i<len; i++)
        _bus->sendData(chars[i]);
}

void Arduino_LT7680::setTextCursorEnable(bool enable)
{
    uint8_t val = enable?0x02:0;

    _registers.GTCCR &= ~(val & 0x10); // Need to disable the graphic cursor
    _registers.GTCCR |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_GTCCR);
    _bus->sendData(_registers.GTCCR);
}

void Arduino_LT7680::setTextCursorBlinking(bool enable)
{
    uint8_t val = enable?1:0;

    _registers.GTCCR &= ~val; 
    _registers.GTCCR |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_GTCCR);
    _bus->sendData(_registers.GTCCR);
}

void Arduino_LT7680::setCursonBlinkTime(uint8_t frameCycles)
{
    while(isBusy());
    _bus->sendCommand(LT7680_BTCR);
    _bus->sendData(frameCycles);    
}

void Arduino_LT7680::setCursorHorizontalSize(uint8_t pixels)
{
    if(pixels == 0)
        pixels = 1;
    
    pixels = (pixels - 1) & 0x1F;

    while(isBusy());
    _bus->sendCommand(LT7680_CURHS);
    _bus->sendData(pixels);
}

void Arduino_LT7680::setCursorVerticalSize(uint8_t pixels)
{
    if(pixels == 0)
        pixels = 1;
    
    pixels = (pixels - 1) & 0x1F;

    while(isBusy());
    _bus->sendCommand(LT7680_CURVS);
    _bus->sendData(pixels);
}

void Arduino_LT7680::SetGraphicMode()
{
    while(isBusy());
    _registers.ICR &= ~0x04;
    _bus->sendCommand(LT7680_ICR);
    _bus->sendData(_registers.ICR);
}

void Arduino_LT7680::setGraphicCursorEnable(bool enable)
{
    uint8_t val = enable?0x10:0;

    _registers.GTCCR &= ~(val & 0x02); // Need to disable the text cursor
    _registers.GTCCR |= val;

    while(isBusy());
    _bus->sendCommand(LT7680_GTCCR);
    _bus->sendData(_registers.GTCCR);
}

void Arduino_LT7680::setGraphicCursorSet(uint8_t set)
{
    uint8_t val = set & 3;
    
    val <<= 3;
    _registers.GTCCR &= ~val;
    _registers.GTCCR |= val; 

    while(isBusy());
    _bus->sendCommand(LT7680_GTCCR);
    _bus->sendData(_registers.GTCCR);
}

void Arduino_LT7680::setGraphicCursorPosition(uint16_t x, uint16_t y)
{
    // Only 13 bits are allowed
    x &= 0x1FFF;
    y &= 0x1FFF;

    while(isBusy());
    _bus->sendCommand(LT7680_GCHP_LO);
    _bus->sendData(x & 0xFF);
    _bus->sendCommand(LT7680_GCHP_HI);
    _bus->sendData((x >> 8) & 0xFF);

    _bus->sendCommand(LT7680_GCVP_LO);
    _bus->sendData(y & 0xFF);
    _bus->sendCommand(LT7680_GCVP_HI);
    _bus->sendData((y >> 8) & 0xFF);
}

void Arduino_LT7680::setGraphicCursonColour0(uint8_t color332)
{
    while(isBusy());
    _bus->sendCommand(LT7680_GCC0);
    _bus->sendData(color332); 
}

void Arduino_LT7680::setGraphicCursonColour1(uint8_t color332)
{
    while(isBusy());
    _bus->sendCommand(LT7680_GCC1);
    _bus->sendData(color332); 
}
