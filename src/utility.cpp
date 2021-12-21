#include "utility.h"

#include "esphome.h"

unsigned crc16(unsigned crc, unsigned char *buf, size_t len) {
  constexpr auto poly = 0xa001;
  while (len--) {
    crc ^= *buf++;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
  }
  return crc;
}
