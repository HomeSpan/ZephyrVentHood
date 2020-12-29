# ZephyrVentHood
 
HomeKit control of a ceiling-mounted <a href="https://zephyronline.com/product/lux-island-range-hood/">Lux Island</a> kitchen vent hood fan by Zephyr.  Runs on an ESP32 device as an Arduino sketch using the Arduino [HomeSpan Library](https://github.com/HomeSpan/HomeSpan).

Hardware required for this project:

* An ESP32 board, such as the [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405?gclid=EAIaIQobChMIh9-Rk4nx7QIVEvDACh0IRwiGEAYYBiABEgJSMPD_BwE)
* A simple 434 MHz RF Transmitter, such as this [RF Link Transmitter](https://www.sparkfun.com/products/10534) from SparkFun
* A simple 434 MHz RF Receiver (to reverse-engineer the remote signals), such as this [RF Link Receiver](https://www.sparkfun.com/products/10532) from SparkFun
* Three pushbuttons (normally-open)

Though the Lux Island Vent Fan is a rather esoteric appliance, the Arduino sketch used to control it is quite generic and can be readily modified to control any simple multi-button RF device that uses fixed codes (for an example of implementing rolling codes with HomeSpan, see my "universal" [Somfy Motorized Window Shade Controller](https://github.com/HomeSpan/SomfyRTS)).  More so, as described fully below, this appliction shows how real-world appliances don't always fit neatly into how HomeKit models the world.  To work around these limitations the sketch uses some internal state variable that allow one HomeKit Service to control another so that the status of the device in your Home App properly reflects the actual status of the appliance.  

### The Zephyr Kitchen Vent Hood with LED Lights

As part of a kitchen remodel we selected a <a href="https://zephyronline.com/product/lux-island-range-hood/">Lux Island</a> vent hood fan by Zephyr to install in the ceiling over a kitchen island cooktop.  The hood contains a very powerful 3-speed fan and sports LED lights to illuminate the cooking area below with 3 levels of brightness.  Since the fan is mounted directly in the ceiling, there are no manual controls or buttons to turn it on or off.  Instead, Zephyr provides this stylistic RF remote control you're supposed to place on your countertop:

![Remote Control](images/zephyr-remote.png)

As shown, it's a simple 4-button remote with buttons for power, the fan, the lights, and a timer control.  Unfortunately, it uses a psuedo-touch sensitive mechanism that is so finicky that there is only a 5% chance that it registers when you press the button.  A replacement remote from the manufacturer performed even worse and it was nearly impossible to get the remote to detect when you pressed one of the buttons.  This project was thus designed to both replace the Zephyr remote with a more robust device, as well as to provide HomeKit control so that you can activate the vent fan and light using Siri (useful when your hands are busy cooking and a pan starts to smoke).

Reverse-engineering the RF signals was quite straigtforward.  Opening the remote revealed it was using a standard 433.92 MHz oscillator (it was printed on the oscillator itself).  With the help of a simple Arduino sketch and an Arduino Mega to perform rapid analog reads, judicious use of the the Arduino Serial Plotter (a terrific feature of the Arduino IDE!), and a simple [434 MHz receiver](https://www.sparkfun.com/products/10532) from SparkFun, I was able to decode the RF signals.

Each button produced a fixed pattern of 20 high/low pulses, each lasting 850𝛍s. The same sequence is repeated 8 times, separated by a 4ms delay.  There are two types of pulses:  one is 230𝛍s high followed by 620𝛍s low, which I arbitrarily took to represent a 0-bit; the other is 620𝛍s high followed by 230𝛍s low, which I therefore took to represent a 1-bit.  Comparing the resulting 20-bit pattern for each button suggested which bits may be related to the address of the device, and which were used to trigger specific functions (power, fan, light, timer).  Since I was not looking to create a universal remote, determining exactly which bits represented the device address and which represented the functions did not matter.  All I needed was to be able to reproduce the exact bit patterns to control the vent hood.

A slightly more complicated Arduino sketch (still using the Arduino Mega), a few pushbuttons, and a simple [434 MHz transmitter](https://www.sparkfun.com/products/10534) from SparkFun and I had myself a fully working remote control to replace the stylistic, though useless, Zephyr remote.

