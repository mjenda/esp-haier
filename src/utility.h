#pragma once

#include "esphome.h"

template <typename Message> byte crc_offset(const Message &message) {
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

// For some reason in captured data when crc16 is computed to FF 29 there is
// a 55 in the middle -> FF 55 29. As a "temporary" workaround just
// hardcore this case. This is ugly hack, but I don't have any idea what is this number.
template <typename Message> bool hackCrc16(Message &message, byte crc16Offset) {
  if (message[crc16Offset + 1] == 0xff && message[crc16Offset + 2] == 0x29) {
    message[crc16Offset + 3] = message[crc16Offset + 2];
    message[crc16Offset + 2] = 0x55;
    return true;
  }
  return false;
}

template <typename Message> void sendData(Message &message) {
  byte offset = crc_offset(message);
  byte crc = getChecksum(message);
  word crc_16 = crc16(0, &(message[2]), offset - 2);

  // Updates the crc
  message[offset] = crc;
  message[offset + 1] = (crc_16 >> 8) & 0xFF;
  message[offset + 2] = crc_16 & 0xFF;

  String hackInformation = "";
  if (hackCrc16(message, offset)) {
    hackInformation = ", but crc16 has been hacked to FF5529";
  }

  Serial.write(message.data(), message.size());

  auto raw = getHex(message);
  ESP_LOGD("EspHaier Utility", "Message sent: %s  - CRC: %X - CRC16: %X%s",
           raw.c_str(), crc, crc_16, hackInformation.c_str());
}
