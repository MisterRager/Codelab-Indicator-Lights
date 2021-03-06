# Codelab for KC2019: ESP32 and PlatformIO

![Indicator Lights Video](https://raw.githubusercontent.com/MisterRager/Codelab-Indicator-Lights/master/assets/lights_action.gif)

The "Internet of Things" is everywhere now, and no piece of hardware makes it easier to build a Thing for the Internet than the ESP32 from Espressif Systems. The follow-up to the wildly popular WiFi-enabled microcontroller, the ESP8266, the ESP32 packs twice the cores (2), adds bluetooth, and greatly expands the number of GPIO's and peripherals available. Procede to the [wiki](https://github.com/MisterRager/Codelab-Indicator-Lights/wiki) and learn how to manipulate neopixel lights, make web requests, and handle multitasking in FreeRTOS.

Pictured is one of the dev boards running the end result of this lab. It connects to a pre-configured WiFi network and then fires off a request for the 7-day forecast for San Francisco. It then assigns color values based on temperature for each of the 12-hour time periods returned for a total of 14 lights.
