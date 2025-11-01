// Copyright (C) 2025 Tube Time. See LICENSE file for license.

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "py32f0xx.h"

SPI_HandleTypeDef hspi2;

#define NOR_CS GPIO_PIN_11 // GPIOA
#define NOR_SI GPIO_PIN_10 // GPIOA, MOSI
#define NOR_SO GPIO_PIN_9  // GPIOA, MISO
#define NOR_CLK GPIO_PIN_2 // GPIOB, CLK

// NOR flash commands
#define CMD_RDID  0x9F // Read JEDEC ID
#define CMD_FREAD 0x0B // Fast read
#define CMD_PE    0x81 // Page erase (256 bytes)
#define CMD_SE    0x20 // Sector erase (4K bytes)
#define CMD_BE32K 0x52 // Block erase (32K bytes)
#define CMD_BE64K 0xD8 // Block erase (64K bytes)
#define CMD_CE    0x60 // Chip erase
#define CMD_PP    0x02 // Page program
#define CMD_RDSR  0x05 // Low status byte
#define CMD_RDSRH 0x35 // High status byte
#define CMD_WREN  0x06 // Set write enable bit

// Status bits
#define STATUS_WIP (1 << 0) // Write in progress bit


void nor_init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI2_CLK_ENABLE();

    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF0_SPI2, .Pin = NOR_SO});
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF0_SPI2, .Pin = NOR_SI});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Alternate = GPIO_AF1_SPI2, .Pin = NOR_CLK});

    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = NOR_CS});
    HAL_GPIO_WritePin(GPIOA, NOR_CS, GPIO_PIN_SET);

    __HAL_RCC_SPI2_FORCE_RESET();
    __HAL_RCC_SPI2_RELEASE_RESET();


    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // By 4 seems reliable, but 2 is faster
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.SlaveFastMode = SPI_SLAVE_FAST_MODE_DISABLE;
    HAL_SPI_Init(&hspi2);

}


void nor_sleep()
{
    HAL_SPI_DeInit(&hspi2);
    __HAL_RCC_SPI2_CLK_DISABLE();

    // Put all NOR flash pins into the high-Z state
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = NOR_SO});
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = NOR_SI});
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = NOR_CLK});
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = NOR_CS});
}


void nor_cs_low()
{
    HAL_GPIO_WritePin(GPIOA, NOR_CS, GPIO_PIN_RESET);
}


void nor_cs_high()
{
    HAL_GPIO_WritePin(GPIOA, NOR_CS, GPIO_PIN_SET);
}


// Call before each erase or program operation
void nor_write_enable()
{
    uint8_t b = CMD_WREN;
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, &b, 1, HAL_MAX_DELAY);
    nor_cs_high();
}


uint16_t nor_read_status()
{
    uint8_t b = CMD_RDSR;
    uint8_t d1, d2;
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, &b, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &d1, 1, HAL_MAX_DELAY);
    nor_cs_high();
    nor_cs_low();
    b = CMD_RDSRH;
    HAL_SPI_Transmit(&hspi2, &b, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &d2, 1, HAL_MAX_DELAY);
    nor_cs_high();
    return ((uint16_t)d2 << 8) | d1;
}


// Page erase. Be sure to enable write before executing this.
void nor_page_erase(uint32_t addr)
{
    uint8_t db[4] = {CMD_PE, addr >> 16, (addr >> 8) & 0xFF, addr & 0xFF}; // Lowest 8 bits should be 0
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, db, 4, HAL_MAX_DELAY);
    nor_cs_high();
}


void nor_page_erase_block(uint32_t addr)
{
    nor_page_erase(addr);
    while (nor_read_status() & STATUS_WIP) {}
}


void nor_page_program(uint32_t addr, uint8_t *data, uint32_t length)
{
    uint8_t db[4] = {CMD_PP, addr >> 16, (addr >> 8) & 0xFF, addr & 0xFF}; // Lowest 8 bits ought to be 0
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, db, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi2, data, length, HAL_MAX_DELAY);
    nor_cs_high();
}


void nor_page_program_block(uint32_t addr, uint8_t *data, uint32_t length)
{
    nor_page_program(addr, data, length);
    while (nor_read_status() & STATUS_WIP) {}
}


uint32_t nor_read_id()
{
    uint8_t b = CMD_RDID; // Read JEDEC ID
    uint32_t dev_id = 0;
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, &b, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, (uint8_t *)&dev_id, 3, HAL_MAX_DELAY);
    nor_cs_high();
    // Mark is "PD64S" "H4BD"-PL2. Puya P25Q64SH.
    // Manufacturer ID = 0x85 = Puya.
    // Memory type ID = 0x60
    // Capacity ID = 0x17
    return dev_id; // Returns 0x176085
}


void nor_read(uint32_t addr, uint8_t *data, uint32_t length)
{
    uint8_t db[4] = {CMD_FREAD, addr >> 16, (addr >> 8) & 0xFF, addr & 0xFF};
    nor_cs_low();
    HAL_SPI_Transmit(&hspi2, db, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, db, 1, HAL_MAX_DELAY); // 8 dummy cycles
    HAL_SPI_Receive(&hspi2, data, length, HAL_MAX_DELAY);
    nor_cs_high();
}

