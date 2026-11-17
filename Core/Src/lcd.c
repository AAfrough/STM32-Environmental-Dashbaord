#include "lcd.h"


/* Generate Enable Pulse */

void LCD_Enable(void)
{
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}


/* Send 4 Bits */

void LCD_Send4Bit(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin,
        (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin,
        (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin,
        (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin,
        (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_Enable();
}


/* Send Command */

void LCD_SendCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);

    LCD_Send4Bit(cmd >> 4);
    LCD_Send4Bit(cmd & 0x0F);

    if (cmd == 0x01 || cmd == 0x02)
        HAL_Delay(2);
    else
        HAL_Delay(1);
}


/* Send Data */

void LCD_SendData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);

    LCD_Send4Bit(data >> 4);
    LCD_Send4Bit(data & 0x0F);

    HAL_Delay(1);
}


/* Initialize LCD */

void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

    LCD_Send4Bit(0x03);
    HAL_Delay(5);

    LCD_Send4Bit(0x03);
    HAL_Delay(1);

    LCD_Send4Bit(0x03);
    HAL_Delay(1);

    LCD_Send4Bit(0x02);
    HAL_Delay(1);

    LCD_SendCommand(0x28);  // 4-bit, 2 lines
    LCD_SendCommand(0x0C);  // Display ON
    LCD_SendCommand(0x06);  // Entry mode
    LCD_SendCommand(0x01);  // Clear display

    HAL_Delay(2);
}


/* Clear Display */

void LCD_Clear(void)
{
    LCD_SendCommand(0x01);
}


/* Set Cursor Position */

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    if (row == 0)
        LCD_SendCommand(0x80 + col);

    else if (row == 1)
        LCD_SendCommand(0xC0 + col);
}


/* Print String */

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_SendData((uint8_t)*str);
        str++;
    }
}
