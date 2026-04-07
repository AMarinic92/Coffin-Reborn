//#include "lcd_i2c.h"
//#include "definitions.h" // Harmony PLIB headers for I2C functions
//
//static uint8_t _addr;
//static uint8_t _backlight = LCD_BACKLIGHT;
//
//static void LCD_I2C_Write4Bits(uint8_t data)
//{
//    SERCOM2_I2C_Write(_addr, &data, 1); // Change SERCOM2_I2C_ to your instance
//}
//
//static void LCD_I2C_PulseEnable(uint8_t data)
//{
//    LCD_I2C_Write4Bits(data | LCD_ENABLE_BIT);
//    SYSTICK_DelayMs(1);
//    LCD_I2C_Write4Bits(data & ~LCD_ENABLE_BIT);
//    SYSTICK_DelayMs(1);
//}
//
//static void LCD_I2C_Send(uint8_t value, uint8_t mode)
//{
//    uint8_t high = value & 0xF0;
//    uint8_t low  = (value << 4) & 0xF0;
//
//    uint8_t data = high | mode | _backlight;
//    LCD_I2C_Write4Bits(data);
//    LCD_I2C_PulseEnable(data);
//
//    data = low | mode | _backlight;
//    LCD_I2C_Write4Bits(data);
//    LCD_I2C_PulseEnable(data);
//}
//
//void LCD_I2C_Command(uint8_t cmd)
//{
//    LCD_I2C_Send(cmd, 0x00);
//}
//
//void LCD_I2C_WriteChar(char c)
//{
//    LCD_I2C_Send(c, 0x01);
//}
//
//void LCD_I2C_Clear(void)
//{
//    LCD_I2C_Command(LCD_CMD_CLEAR);
//    SYSTICK_DelayMs(2);
//}
//
//void LCD_I2C_Home(void)
//{
//    LCD_I2C_Command(LCD_CMD_HOME);
//    SYSTICK_DelayMs(2);
//}
//
//void LCD_I2C_SetCursor(uint8_t col, uint8_t row)
//{
//    static const uint8_t offsets[] = {0x00, 0x40, 0x14, 0x54};
//    if (row > 1) row = 1;
//    LCD_I2C_Command(LCD_CMD_SETDDRAMADDR | (col + offsets[row]));
//}
//
//void LCD_I2C_Print(const char *str)
//{
//    while (*str)
//        LCD_I2C_WriteChar(*str++);
//}
//
//void LCD_I2C_Initialize(uint8_t i2cAddress)
//{
//    _addr = i2cAddress;
//
//    SYSTICK_DelayMs(50); // wait for LCD to power up
//
//    LCD_I2C_Send(0x30, 0x00);
//    SYSTICK_DelayMs(5);
//    LCD_I2C_Send(0x30, 0x00);
//    SYSTICK_DelayMs(5);
//    LCD_I2C_Send(0x20, 0x00); // 4-bit mode
//
//    LCD_I2C_Command(LCD_CMD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
//    LCD_I2C_Command(LCD_CMD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
//    LCD_I2C_Command(LCD_CMD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
//    LCD_I2C_Clear();
//}
