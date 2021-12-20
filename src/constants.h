#pragma once

// Updated read offset

#define MODE_OFFSET 14
#define MODE_MSK 0xF0
#define MODE_AUTO 0x00
#define MODE_DRY 0x40
#define MODE_COOL 0x20
#define MODE_HEAT 0x80
#define MODE_FAN 0xC0
#define FAN_MSK 0x0F
#define FAN_LOW 0x03
#define FAN_MID 0x02
#define FAN_HIGH 0x01
#define FAN_AUTO 0x05

#define HORIZONTAL_SWING_OFFSET 19
#define HORIZONTAL_SWING_CENTER 0x00
#define HORIZONTAL_SWING_MAX_LEFT 0x03
#define HORIZONTAL_SWING_LEFT 0x04
#define HORIZONTAL_SWING_MAX_RIGHT 0x06
#define HORIZONTAL_SWING_RIGHT 0x05
#define HORIZONTAL_SWING_AUTO 0x07

#define VERTICAL_SWING_OFFSET 13
#define VERTICAL_SWING_MAX_UP 0x02
#define VERTICAL_SWING_UP 0x04
#define VERTICAL_SWING_CENTER 0x06
#define VERTICAL_SWING_DOWN 0x08
#define VERTICAL_SWING_HEALTH_UP 0x01
#define VERTICAL_SWING_HEALTH_DOWN 0x03
#define VERTICAL_SWING_AUTO 0x0C

#define TEMPERATURE_OFFSET 22

#define STATUS_DATA_OFFSET 17 // Purify/Quiet mode/OnOff/...
#define POWER_BIT (0)
#define PURIFY_BIT (1)
#define QUIET_BIT (3)
#define AUTO_FAN_MAX_BIT (4)

#define SET_POINT_OFFSET 12

// Another byte
#define SWING 27
#define SWING_OFF 0
#define SWING_VERTICAL 1
#define SWING_HORIZONTAL 2
#define SWING_BOTH

#define LOCK 28
#define LOCK_ON 80
#define LOCK_OFF 00

// Updated read offset

#define FRESH 31
#define FRESH_ON 1
#define FRESH_OFF 0

// Updated read offset

#define COMMAND_OFFSET 9
#define RESPONSE_POLL 2

#define CRC_OFFSET(message) (2 + message[2])

// Control commands
#define CTR_POWER_OFFSET 13
#define CTR_POWER_ON 0x01
#define CTR_POWER_OFF 0x00

#define POLY 0xa001

// temperatures supported by AC system
#define MIN_SET_TEMPERATURE 16
#define MAX_SET_TEMPERATURE 30

// if internal temperature is outside of those boundaries, message will be
// discarded
#define MIN_VALID_INTERNAL_TEMP 10
#define MAX_VALID_INTERNAL_TEMP 50