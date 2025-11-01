// Copyright (C) 2025 Tube Time. See LICENSE file for license.

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "py32f0xx.h"

ADC_HandleTypeDef hadc;


void adc_init(void)
{
    __HAL_RCC_ADC_FORCE_RESET();
    __HAL_RCC_ADC_RELEASE_RESET();
    __HAL_RCC_ADC_CLK_ENABLE();

    // Turn on battery voltage monitoring resistor divider
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_OD, .Pin = GPIO_PIN_12});
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

    // Turn on ADC channel
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_0}); // USB V
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_1}); // Batt V
    HAL_GPIO_Init(GPIOF, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_2}); // Chg I

    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8; // 6MHz. Also try ADC_CLOCK_ASYNC_HSI_DIV1
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc.Init.LowPowerAutoWait = ENABLE; // was disable
    hadc.Init.DMAContinuousRequests = DISABLE;
    // tADC = (3.5 + 13.5) * (1/12MHz) = 1.4167us > 1us min (ok)
    hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5; // Was 13CYCLES_5, trying longest for now.
    HAL_ADC_Init(&hadc);

    HAL_ADCEx_Calibration_Start(&hadc); // Run calibration before HAL_ADC_Start()
}


uint32_t get_adc(int channel)
{
    uint32_t adc_val;
    ADC_ChannelConfTypeDef sconfig = {0};

    // Select the channel
    sconfig.Channel = channel;
    sconfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sconfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5; 
    HAL_ADC_ConfigChannel(&hadc, &sconfig);

    // Do the conversion
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
    adc_val = HAL_ADC_GetValue(&hadc);
    HAL_ADC_Stop(&hadc);

    // Disable the channel. This is required by all PY32 devices
    // but poorly documented and not done in any sample code.
    sconfig.Rank = ADC_RANK_NONE;
    HAL_ADC_ConfigChannel(&hadc, &sconfig);
   
    return adc_val;
}


void adc_sleep()
{
    // Shut off battery monitoring resistor divider enable
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_12}); // EN_MON_BAT

    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_0}); // MON_USB
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_2}); // MON_F2
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_3}); // MON_F1
    HAL_GPIO_Init(GPIOF, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_3}); // EN_MON_F1
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_4}); // EN_MON_F2
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_4}); // EN_F2
    HAL_GPIO_Init(GPIOF, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_4}); // EN_F1

    HAL_ADC_DeInit(&hadc);
    __HAL_RCC_ADC_CLK_DISABLE();
}

