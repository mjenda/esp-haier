#pragma once

#include "esphome.h"

byte getChecksum(const byte *message, size_t size) {
  byte position = CRC_OFFSET(message);
  byte crc = 0;

  if (size < (position)) {
    ESP_LOGE("Control", "frame format error (size = %d vs length = %d)", size,
             message[2]);
    return 0;
  }

  for (int i = 2; i < position; i++)
    crc += message[i];

  return crc;
}

unsigned crc16(unsigned crc, unsigned char *buf, size_t len) {
  while (len--) {
    crc ^= *buf++;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
  }
  return crc;
}

String getHex(const byte *message, byte size) {

  String raw;

  for (int i = 0; i < size; i++) {
    raw += " " + String(message[i]);
  }
  raw.toUpperCase();

  return raw;
}

void sendData(byte *message, byte size) {
  byte crc_offset = CRC_OFFSET(message);
  byte crc = getChecksum(message, size);
  word crc_16 = crc16(0, &(message[2]), crc_offset - 2);

  // Updates the crc
  message[crc_offset] = crc;
  message[crc_offset + 1] = (crc_16 >> 8) & 0xFF;
  message[crc_offset + 2] = crc_16 & 0xFF;

  Serial.write(message, size);

  auto raw = getHex(message, size);
  ESP_LOGD("EspHaier Utility", "Message sent: %s  - CRC: %X - CRC16: %X",
           raw.c_str(), crc, crc_16);
}
