This is a box with sensors for an hypothetical mushroom farm for a school assignment

It can be mounted on the wall. It monitors temperature, humidity and light intensity through use of the dht22 sensor and a ldr sensor
It uses LED's to indicate if the reading is in the optimal range(green) or not(red).
There is a testbutton on the device as well as in the Node Red dashboard to light up all leds for 2 seconds
Readings can be seen in the serial monitor, on a webserver via http or on Node Red dashboard via MQTT and, if commented in, via WebBLE
(WebBLE is commented out due to not enough memory)
(The ldr sensor reading does not directly correspond to an amount of lumen, but i did not have a photo diode to measure more accurately)
It can receive updates over the air vie ElegantOTA