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

The Zephyr Vent Hood has both a fan and a light, which should in principle be easy to replicate as a single Accessory with one Fan Service (with 3 speeds) and one Light Bulb Service (with 3 levels of brightness).  Unfortunately, the Zephyr Hood controls are not as clean, and the the Fan and Light operations are somewhat linked together as follows:

If both the fan and light are off:

* a press of the power button turns on the fan to low speed; or
* a press of the fan button turns on the fan to low speed; or
* a press of the light button turns on the light to high brightness.

Pressing the light button when the light is already on cycles the brightness from high → medium → low → off, and then back to high.  Pressing the fan button when the fan is already running cycles the speed from low → medium → high, and then back to low.  **There is *no* off setting when pressing the fan button**.  Instead, to turn the fan off, you need to press the power button.  But that also turns off the light, which leads to a linkage between the fan and light that is more complex than the idealized standalone Fan and Light Bulb Service in HomeKit.  For example, if the fan is off *and* the light is off, the fan comes on when you press the power button.  But if the light is already on and you press the power button, the light turns off, and nothing happens to the fan.

A second complication is that the Zephyr only allows you to cycle through speeds and brightness levels.  There is no remote control command that directs the Zephyr to set the fan to a specific speed or the light to a specific brightness.

Fortunately, a little bit of extra logic that takes advantage of Homespan's flexible service-oriented structure (service as in HomeKit Service, *not* as in client/servers), and some practical compromise, is all that is needed to solve for these complications.

### Potential Solutions

There are at least three ways to solve the very common problem of representing a multi-speed fan or dimmable light in HomeKit even when the device only permits cycing through different levels in a fixed order:

1. Create logic that sends repeated commands to cycling through multiple fan speeds or multiple levels of brightness until the speed or brightness matches the value  set by the user Home App matches the speed.  The problems with this are -
  1. If just one RF command is dropped or missed, the real-world device will get out of sync with what the code thinks the speed or brightness is.
  1. The device will need to "flicker" through multiple levels as it cycles to the desired state.  For a light, this can be very annoying.  For a fan, changing the speed from medium to low by rapidly cyling first through high can be very annoying *and* can damage the motor (or at least shorten its life).
1. Use HomeSpan's emulated pushbutton features to replicate the real-work functionality of the remote buttons.  A single press of a "Speed" tile in the Home App could cycle the fan speed.  A single press of a "Brightness" tile in the Home App could cycle the brightness.  The problem with this method is that although it works well if you operate the tiles in the Home App, it does not translate well with Siri.  Telling Siri to increase the brightness does not work, since Siri has no idea that the brightness tile you creates is really related to the brightness of the light.  Instead you would need to say "Siri, turn ON vent light brightness," which is quite clunky.  To get around this you could create a HomeKit scene or Siri Shortcut called "Decrease Vent Light Brightness" that would trigger the brightness tile, but this is also rather clunky (I tried this in practice, and it really is) since there is no equivalent "Increase Vent Light Brightness" button becuase the Zephy light only cycles high → medium → low → off, and then back to high.
1. Don't bother implementing speed and brightness controls from within HomeKit.

### 
