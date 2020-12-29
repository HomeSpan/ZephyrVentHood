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

### The RF Signals

Opening the remote revealed it was using a standard 433.92 MHz oscillator (it was printed on the oscillator itself).  Using an Arduino Mega, the Arduino Serial Plotter (a terrific feature of the Arduino IDE!), and a 434 MHz receiver allows or an easy reverse-engineering of the RF signals.  

Each button on the remote produces a 8 repeats of a fixed pattern of twenty high/low pulses. Individual high/low pulses are 850𝛍s in duration, and there is a delays of 4ms between repeats of the pattern.  There are two types of pulses:  one is 230𝛍s high followed by 620𝛍s low, which can be arbitrarily assigned to represent a 0-bit; the other is 620𝛍s high followed by 230𝛍s low, which would represent a 1-bit.  Each button can therefore be represented by a 5 hex-digits, such as 0x51388.

### Generating the RF Codes

The sketch uses HomeSpan's `RFControl` library to output a digital version of a 20-bit pattern on a pin that is connected to the input of a 434 MHz transmitter.  When a certain button press is required, the code loads 20 high/low pulses into the ESP32's dedicated RMT (Remote Control) Memory using either an `add(230,620)` to represent a 0-bit or an `add(620,230)` to represent a 1-bit.  For the last bit to be encoded, the sketch adds an additonal 4000 ticks to the second argument, yielding either an `add(230,4620)` or an `add(620,4230)` depending on whether the last bit was a 0 or 1.  The sketch then calls `start(8,1)` to directs the ESP32 to generate 8 repeats of this pattern with 1 tick = 1 microsecond.

### Real-World Complications in the Idealized-World of HomeKit

