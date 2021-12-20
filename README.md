# ESP Haier
ESP Haier is a project to use a **ESP8266** to control Wifi enabled Haier Air Conditioner with ESPHome and Home Assistant.

# Installation
Create *secrets.yaml* in root directory and insert your network details there:
```
wifi-ssid: YOUR_SSID
wifi-password: WIFI_PASSWORD
ip-address: IP_ADDRESS_TO_YOUR_UNIT
ip-gateway: GATEWAY
ip-subnet: SUBNET
```
And then:
```
pip3 install esphome
esphome run esphaier.yaml
```

Home Assisant will recognize your unit as climate device.

As a controller I used Wemos D1 Mini with USB cable soldered directly to Wemos:
- Red   -> 5V
- Black -> GND
- Green -> RX
- White -> TX

# Tested devices
> Haier Flexis White Matt, firmare R_1.0.00/e_2.5.14
# Credits
* [First author](https://github.com/MiguelAngelLV/esphaier)
* [Second author](https://github.com/albetaCOM/esp-haier)
