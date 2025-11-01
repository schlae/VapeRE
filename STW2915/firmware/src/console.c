// Copyright (C) 2025 Tube Time. See LICENSE file for license.

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "py32f0xx.h"

#include "adc.h"
#include "lcd.h"
#include "nor.h"

UART_HandleTypeDef huart1;


// Initialize UART on pins PA2 and PA3. On the STW2915, these conflict
// with the heater feedback ADC inputs so don't use both at once.
void uart_init()
{
    __HAL_RCC_USART1_CLK_ENABLE();
    // PA2 = USART1_TX out
    // PA3 = USART1_RX in
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF1_USART1, .Pin = GPIO_PIN_2});
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF1_USART1, .Pin = GPIO_PIN_3});
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 500000; // Exact and fast, unlike 115200 which is not exact.
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart1);
}


// Crude console mostly meant for transferring data to/from the onboard NOR flash.
// 500000 8N1 no flow control
void console(void)
{
    uart_init();
    HAL_Delay(100);
    uint32_t start_addr, data_len;
    char cmd;
    uint8_t t[2048];
    char s[80];
    uint16_t status;

 //   uint32_t id = nor_read_id();

 //   sprintf(s, "Welcome, enter a single letter command.\r\n");
    HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY);

    while(1) {
        HAL_UART_Receive(&huart1, (uint8_t *)&cmd, 1, HAL_MAX_DELAY);
        switch(cmd) {
        case 'D': 
            // Dump contents of SPI flash over UART
            start_addr = 0;
            for (int i = 0; i < 4096; i++) {
                nor_read(start_addr, t, sizeof(t));
                start_addr += sizeof(t);
                HAL_UART_Transmit(&huart1, t, sizeof(t), HAL_MAX_DELAY);
            }
            break;
        case 'r': // Read a 256-byte page
            HAL_UART_Receive(&huart1, (uint8_t *)&start_addr, 4, HAL_MAX_DELAY);
            nor_read(start_addr, t, 256);
            HAL_UART_Transmit(&huart1, t, 256, HAL_MAX_DELAY);
            break;
        case 'f': // Human readable dump of first few bytes of NOR
            nor_read(0, t, 8);
            sprintf(s, "%.2x %.2x %.2x %.2x\r\n", t[0], t[1], t[2], t[3]);
            HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY);
            sprintf(s, "%.2x %.2x %.2x %.2x\r\n", t[4], t[5], t[6], t[7]);
            HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY);
            break;
        case 's': // Returns human readable status word
            status = nor_read_status();
            sprintf(s, "%.4x\r\n", status);
            HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY);
            break;
        case 'w': // Enable writes
            nor_write_enable();
            break;
        case 'E': // Erase a page
            HAL_UART_Receive(&huart1, (uint8_t *)&start_addr, 4, HAL_MAX_DELAY);
            nor_page_erase_block(start_addr);
            s[0] = 'A';
            HAL_UART_Transmit(&huart1, s, 1, HAL_MAX_DELAY);
            break;
        case 'P': // Program up to a page
            HAL_UART_Receive(&huart1, (uint8_t *)&start_addr, 4, HAL_MAX_DELAY);
            HAL_UART_Receive(&huart1, (uint8_t *)&data_len, 4, HAL_MAX_DELAY);
            s[0] = 'B';
            if (data_len <= 256) {
                HAL_UART_Receive(&huart1, t, data_len, HAL_MAX_DELAY);
                nor_page_program_block(start_addr, t, data_len);
                s[0] = 'A';
            }
            HAL_UART_Transmit(&huart1, s, 1, HAL_MAX_DELAY);
            break;
        case 'q': // Exit and return to main program
            return;
            break;
        default:
            break;
        }
    }
}
