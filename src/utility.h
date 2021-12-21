#pragma once

#include "esphome.h"

template <typename Message> uint8_t crc_offset(const Message &message) {
  return message[2] + 2u;
}

template <typename Message> byte getChecksum(const Message &message) {
  byte position = crc_offset(message);
  byte crc = 0;

  if (message.size() < (position)) {
    ESP_LOGE("Control", "frame format error (size = %d vs length = %d)",
             message.size(), message[2]);
    return 0;
  }

  for (int i = 2; i < position; i++)
    crc += message[i];

  return crc;
}

unsigned crc16(unsigned crc, unsigned char *buf, size_t len);

template <typename Message> String getHex(const Message &message) {

  String raw;

  for (int i = 0; i < message.size(); i++) {
    raw += " " + String(message[i]);
  }
  raw.toUpperCase();

  return raw;
}

template <typename Message> void sendData(Message &message) {
  byte offset = crc_offset(message);
  byte crc = getChecksum(message);
  word crc_16 = crc16(0, &(message[2]), offset - 2);

  // Updates the crc
  message[offset] = crc;
  message[offset + 1] = (crc_16 >> 8) & 0xFF;
  message[offset + 2] = crc_16 & 0xFF;

  Serial.write(message.data(), message.size());

  auto raw = getHex(message);
  ESP_LOGD("EspHaier Utility", "Message sent: %s  - CRC: %X - CRC16: %X",
           raw.c_str(), crc, crc_16);
}
