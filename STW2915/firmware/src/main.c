// Copyright (C) 2025 Tube Time. See LICENSE file for license.

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "py32f0xx.h"

#include "adc.h"
#include "console.h"
#include "lcd.h"
#include "nor.h"
#include "sserif13.h"


EXTI_HandleTypeDef hexti;

volatile bool ShutdownFlag = false;


void SysTick_Handler()
{
    HAL_IncTick();
}


void error_handler(void)
{
    while(1) {}
}


void EXTI4_15_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&hexti);
}


// Configure the clock to use the PLL at the full 48MHz.
void system_clock_config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          // Enable HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          // HSI clock 1 division
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; // Config HSI 24MHz Calibration
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         // Disable HSE
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         // Disable LSI
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         // Disable LSE
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                      // Enable PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    error_handler();
  }

  // Configure Clock 
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // System clock selection PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     // AHB clock 1 division
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      // APB clock 1 division
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) // Flash latency is 0 for <=24MHz, 1 for 48MHz.
  {
    error_handler();
  }
}


// Switch back to the HSI so the PLL can shut down
void slowclock()
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  // Configure Clock 
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; // System clock selection PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     // AHB clock 1 division
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      // APB clock 1 division
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) // Flash latency is 0 for <=24MHz, 1 for 48MHz.
  {
    error_handler();
  }

} 


void PushbuttonCallback(void)
{
    // Signal routine in main loop to check for power down state
    // Or reset it since this routine is called on wake-up
    ShutdownFlag = !ShutdownFlag;
}


void gpio_init()
{
    EXTI_ConfigTypeDef exti_config = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    // Turn on 3.3V to LCD, etc. 
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = GPIO_PIN_8});
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_OUTPUT_PP, .Pin = GPIO_PIN_8});
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    
    HAL_GPIO_Init(GPIOF, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_INPUT, .Pin = GPIO_PIN_1}); // Charge ind

    // GPIOA.0 is the USB voltage divider. Could set up an interrupt on that before sleep?
    // This would be EXTI0. Int handler would be EXTI0_1_IRQHandler. Look for rising edge.
    // Allows detection on plugging in a USB charger.

    // Pushbutton switch with external pullup
    // External pullup is oddly specific value: 464K
    // Could be measured with ADC if you turned on the internal pulldown of 30K-70K.
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_INPUT | GPIO_MODE_IT_FALLING,
                                             .Pull = GPIO_NOPULL, .Pin = GPIO_PIN_7});
    exti_config.Line = EXTI_LINE_7;
    exti_config.Mode = EXTI_MODE_INTERRUPT;
    exti_config.Trigger = EXTI_TRIGGER_FALLING; // Could do RISING_FALLING as well.
    exti_config.GPIOSel = EXTI_GPIOA;
    HAL_EXTI_SetConfigLine(&hexti, &exti_config);

    HAL_EXTI_RegisterCallback(&hexti, HAL_EXTI_COMMON_CB_ID, PushbuttonCallback);

    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}


void gpio_sleep()
{    
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_ANALOG, .Pin = GPIO_PIN_13}); // SWDIO
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // Turn off 3V3
}

#define EXT_VREG 300 // 3.00 volts
#define ADC_MAX_CODE 4095


// Convert ADC reading into fixed-point decimal voltage based on input resistor divider
int volt_conv(int raw, int rtop, int rbot)
{
    // (((raw / 4095) * 3.0) / Rbot) * (Rbot + Rtop)
    // (raw * 3.0) * (Rbot + Rtop) / ( 4095 * Rbot )
    int numerator = raw * EXT_VREG * (rbot + rtop);
    int denominator = ADC_MAX_CODE * rbot;
    int half = denominator / 2;
    return (numerator + half) / denominator;
}

// ADC voltage reference calibration value
// Typically 0x1208 (most significant word)
// I think they accidentally encoded this as a decimal number.
//volatile const uint32_t VREFCAL = *(uint32_t *)0x1FFF0E20;

#define TEMP_MIN 300 // Tenths of a degree C
#define TEMP_MAX 850
#define CAL_VDD 330

int temp_conv(int raw)
{
    // Values from system area for calibration of the temp sensor.
    // Note these are measured at VDD=3.3V and must be corrected
    // since we are running at 3.0V, which is our analog reference voltage.
    // Value at 30C should be around 0.76/3.3*4095 = 943.
    // Value at 85C should be around 0.8975/3.3*4095 = 1114.
    // 30C -> 0.76/3.0 * 4095 = 1037
    // 85C -> 0.8975/3.0 * 4095 = 1225
    // Tj seems to be about 5.3C above ambient.
    // See AN1038E for details
    volatile const int TSCAL1 = *(uint32_t *)0x1FFF0F14;
    volatile const int TSCAL2 = *(uint32_t *)0x1FFF0F18;

    // (TEMP_MAX-TEMP_MIN) * (raw - (TSCAL1 * CAL_VDD / EXT_VREG)) / ((CAL_VDD / EXT_VREG) * (TSCAL2 - TSCAL1)) + TEMP_MIN
    // (TEMP_MAX-TEMP_MIN) * (EXT_VREG * raw - TSCAL1 * CAL_VDD) / (CAL_VDD * (TSCAL2 - TSCAL1)) + TEMP_MIN
    int numerator = (TEMP_MAX - TEMP_MIN) * (EXT_VREG * raw - TSCAL1 * CAL_VDD);
    int denominator = CAL_VDD * (TSCAL2 - TSCAL1);
    int half = denominator / 2;
    return TEMP_MIN + (numerator + half) / denominator;
}


// Use piecewise linear approximation to get battery state of charge
uint32_t bat_soc(uint32_t voltage)
{
    if (voltage >= 425) {
        return 100;
    } else if ((voltage >= 400) && (voltage < 425)) {
        return ((voltage - 400) * 10 / 25) + 90;
    } else if ((voltage >= 375) && (voltage < 400)) {
        return ((voltage - 375) * 60 / 25) + 30;
    } else if ((voltage >= 330) && (voltage < 375)) {
        return ((voltage - 330) * 30 / 45);
    } else {
        return 0;
    }
}


int main (void)
{
    volatile uint32_t adc_val1, adc_val2, adc_val3, adc_val4;
    uint8_t testbuf[256];
    PWR_StopModeConfigTypeDef stop_config = {0};

    HAL_Init();
    system_clock_config();
    gpio_init();
    st7789_init();
    nor_init();
    adc_init();
  
    st7789_bitblt_nor(0, 0, 0); // x=y=0, address in NOR=0

#if 0
    // Open serial console for debug access to NOR flash, among other things.
    console();
#endif

    HAL_Delay(100); // Delay in case we broke SWDIO

    while(1) {
#if 1
        st7789_fill(0, 0, 283, 2, 0x003F);
        adc_val1 = temp_conv(get_adc(ADC_CHANNEL_TEMPSENSOR));
        adc_val2 = volt_conv(get_adc(ADC_CHANNEL_9), 10, 10); // Batt voltage
        adc_val3 = volt_conv(get_adc(ADC_CHANNEL_0), 300, 200); // USB voltage
        adc_val4 = bat_soc(adc_val2);
        sprintf((char *)testbuf, "Temp: %ld. Batt: %ld. USB: %ld.", adc_val1, adc_val2, adc_val3);
        font_string(0, 0, (char *)testbuf, strlen((char *)testbuf), 0xffff, 0x0000, &sserif13, false);
        sprintf((char *)testbuf, "CHARGE ST: %d, SOC: %ld%%   ", HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1), adc_val4);
        font_string(0, 15, (char *)testbuf, strlen((char *)testbuf), 0xffff, 0x0000, &sserif13, false);
#endif
        HAL_Delay(500);

        // Check for shutdown
        if (ShutdownFlag) {        
            HAL_Delay(1000); // Delay for Joulescope measurement

            // Tell each module to sleep
            nor_sleep();
            st7789_sleep();
            adc_sleep();
            gpio_sleep();

            // Go to stop 0 mode
            stop_config.LPVoltSelection = PWR_STOPMOD_LPR_VOLT_SCALE2;
            stop_config.WakeUpHsiEnableTime = PWR_WAKEUP_HSIEN_AFTER_MR;
            stop_config.FlashDelay = PWR_WAKEUP_FLASH_DELAY_2US; // was 5
            HAL_PWR_ConfigStopMode(&stop_config);
            HAL_SuspendTick();
            slowclock();

            HAL_Delay(1000); // Delay for Joulescope measurement

            // Note that this will not work while using the JTAG debugger.
            // 56uA. 1uA for LP4068. 5uA for CJ6330 LDO.
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

            // ZZZZZZZZZZZZ

            // Wake up
            system_clock_config();
            HAL_ResumeTick();
            HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){.Mode = GPIO_MODE_AF_PP, .Pin = GPIO_PIN_13,
                                                     .Speed = GPIO_SPEED_FREQ_VERY_HIGH, .Pull = GPIO_PULLUP}); // SWDIO
            // Turn on modules again
            gpio_init();
            st7789_init();
            nor_init();
            adc_init();
            st7789_bitblt_nor(0, 0, 0); // x=y=0, address in NOR=0
            ShutdownFlag = false;
        }
    }
}
