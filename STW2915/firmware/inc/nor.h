#ifndef NOR_H
#define NOR_H

void nor_init();
void nor_sleep();

void nor_write_enable();
void nor_page_erase(uint32_t addr);
void nor_page_erase_block(uint32_t addr);
void nor_page_program(uint32_t addr, uint8_t *data, uint32_t length);
void nor_page_program_block(uint32_t addr, uint8_t *data, uint32_t length);
uint32_t nor_read_id();
uint16_t nor_read_status();
void nor_read(uint32_t addr, uint8_t *data, uint32_t length);

#endif
