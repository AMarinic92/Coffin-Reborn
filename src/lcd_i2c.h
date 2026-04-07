/* ************************************************************************** */
/** 
  @Company
    Your Company Name

  @File Name
    lcd_i2c.h

  @Summary
    I2C 1602 LCD Driver Header File.

  @Description
    This file provides an interface to the HD44780-compatible 1602 LCD
    using an I2C I/O expander (PCF8574/PCF8574A). It is designed for 
    use with Microchip SAME51J20A and MPLAB Harmony 3 I2C PLIB drivers.
 */
/* ************************************************************************** */

#ifndef _LCD_I2C_H    /* Guard against multiple inclusion */
#define _LCD_I2C_H

/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>

/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */

/** 
  @Summary
    LCD control bit masks and command codes.
*/
#define LCD_BACKLIGHT           0x08
#define LCD_ENABLE_BIT          0x04

#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRYMODESET    0x04
#define LCD_CMD_DISPLAYCONTROL  0x08
#define LCD_CMD_FUNCTIONSET     0x20
#define LCD_CMD_SETDDRAMADDR    0x80

#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON           0x04
#define LCD_CURSOROFF           0x00
#define LCD_BLINKOFF            0x00

#define LCD_2LINE               0x08
#define LCD_5x8DOTS             0x00
#define LCD_4BITMODE            0x00

/* ************************************************************************** */
/* Section: Data Types                                                        */
/* ************************************************************************** */

/**
  @Summary
    LCD configuration structure.

  @Description
    Holds the LCD?s I2C address and configuration state.
*/
typedef struct
{
    uint8_t address;        /**< I2C address of the LCD module */
    bool backlightEnabled;  /**< Backlight on/off flag */
} LCD_I2C_OBJ;

/* ************************************************************************** */
/* Section: Interface Functions                                               */
/* ************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

/**
  @Function
    void LCD_I2C_Initialize(uint8_t i2cAddress);

  @Summary
    Initializes the LCD over I2C.

  @Description
    Sends the proper initialization sequence for HD44780 4-bit mode via the 
    I2C backpack (PCF8574). Must be called after I2C peripheral is initialized.

  @Precondition
    I2C peripheral must be initialized and enabled.

  @Parameters
    @param i2cAddress - I2C address of the LCD backpack (e.g., 0x27 or 0x3F).

  @Returns
    None.
*/
void LCD_I2C_Initialize(uint8_t i2cAddress);

/**
  @Function
    void LCD_I2C_Clear(void);

  @Summary
    Clears the LCD display.
*/
void LCD_I2C_Clear(void);

/**
  @Function
    void LCD_I2C_Home(void);

  @Summary
    Moves the cursor to the home position (0,0).
*/
void LCD_I2C_Home(void);

/**
  @Function
    void LCD_I2C_SetCursor(uint8_t col, uint8_t row);

  @Summary
    Sets the cursor to a specific column and row.
*/
void LCD_I2C_SetCursor(uint8_t col, uint8_t row);

/**
  @Function
    void LCD_I2C_Print(const char *str);

  @Summary
    Prints a null-terminated string to the LCD.
*/
void LCD_I2C_Print(const char *str);

/**
  @Function
    void LCD_I2C_WriteChar(char c);

  @Summary
    Writes a single character to the LCD.
*/
void LCD_I2C_WriteChar(char c);

/**
  @Function
    void LCD_I2C_Command(uint8_t cmd);

  @Summary
    Sends a raw command byte to the LCD controller.
*/
void LCD_I2C_Command(uint8_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* _LCD_I2C_H */

/* *****************************************************************************
 End of File
 */


