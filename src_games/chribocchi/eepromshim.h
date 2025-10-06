#pragma once

void eeprom_busy_wait() {};

uint8_t eeprom_read_byte(const uint8_t * eepAddr) {return 0;};

uint16_t eeprom_read_word(const uint16_t * eepAddr) {return 0;};

uint32_t  eeprom_read_dword(const uint32_t * eepAddr) {return 0;};

void eeprom_read_block(void *p, const void * eepAddr, size_t n) {};

void eeprom_write_byte(uint8_t * eepAddr, uint8_t val) {};

void eeprom_write_word(uint16_t *eepAddr, uint16_t val) {};

void eeprom_write_dword(uint32_t *eepAddr,  uint32_t val) {};

void eeprom_write_block(const void *p, void * eepAddr, size_t n) {};