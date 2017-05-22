# Weather Console on a ESP32 with OLED

This is source code to the Weather Console running Arduino on an ESP32 with a 128x64 SSD1306 OLED display. The board using is a rip off copy of the original <a href="https://www.tindie.com/products/lspoplove/d-duino-32esp32-and-096oled-display/">D-duino-32 from Tindie</a>. However, I believe the electronics are mostly identical and therefore should not have any issue running on the genuine one.

The OLED library using here is the <a href="https://github.com/squix78/esp8266-oled-ssd1306">ESP8266 and ESP32 Oled Driver for SSD1306 display by Daniel Eichhorn and Fabrice Weinberg</a>. One can of course chose to use other SSD1306 libraries such as the AdaFruit one. Just remember to change the SDA, SCL pins to the correct ones.

## Important things on I2C

The layout of the D-duino-32 board uses pin 4 for SCL and pin 5 for SDA of the I2C communication with the OLED display. Also, this 128x64 OLED has the I2C address of 0x3C. Note that example code in the AdaFruit's library uses 0x3D for the 128x64 OLED and you must change this to 0x3C for the display to work. The AdaFruit's library also assume a defult I2C pin configuration and that must also be changed to the correct SCL and SDA pins.

## Weather information

The weather information is obtained by subscrbing to some MQTT messages via a MQTT broker. The code here is customized to my environment and you shall need to change those to fit yours.

## Time synchornization

The code here has two different ways to synchornise time - NTP and MQTT. I have disabled the NTP part and uses a special MQTT topic *sensornet/time/timestamp* to get the epoch from my time server at home. I have ran into problem of screwing up the clock if NTP did not get a valid response back in time. I have yet have time to resolve this issue.
