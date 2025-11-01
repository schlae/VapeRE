#ifndef ADC_H
#define ADC_H

void adc_init(void);
uint32_t get_adc(int channel);
void adc_sleep();

#endif
