# ZephyrVentHood
 
HomeKit control of a ceiling-mounted <a href="https://zephyronline.com/product/lux-island-range-hood/">Lux Island</a> kitchen vent hood fan by Zephyr.

Built with [HomeSpan](https://github.com/HomeSpan/HomeSpan)

---

(This section under construction)

As part of a kitchen remodel we had a installed a <a href="https://zephyronline.com/product/lux-island-range-hood/">Lux Island</a> vent hood fan by Zephyr in the ceiling over a kitchen island cooktop.  The hood contains a very powerful 3-speed fan and sports LED lights to illuminate the cooking area below with 3 levels of brightness.  Since the fan is mounted directly in the ceiling, there are no manual controls or buttons to turn it on or off.  Instead, Zephyr provides this stylistic RF remote control you're supposed to place on your countertop:

![Remote Control](images/zephyr-remote.png)

As shown, it's a simple 4-button remote with buttons for power, the fan, the lights, and a timer control.  Unfortunately, it uses a psuedo-touch sensitive mechanism that is so finicky that there is only a 5% chance that it registers when you press the button.  A replacement remote from the manufacturer performed even worse and it was nearly impossible to get the remote to detect when you pressed one of the buttons.  Rather than ask the manufacturer for another replacement, I decided to try my hand at creating my own RF remote.

Reverse-engineering the RF signals was quite straigtforward.  Opening the remote revealed it was using a standard 433.92 MHz oscillator.  With the help of a simple Arduino sketch to perform rapid analog reads, judicious use of the the Arduino Serial Plotter (a terrific feature of the Arduino IDE!), and a simple [434 MHz receiver](https://www.sparkfun.com/products/10532) from SparkFun, I was able to decode the RF signals.

Each button produced a fixed pattern of 20 850𝛍pulses, repeated 8 times.  

first with an Arduino, then with an Arduino hosting a small web-server that connected to HomeKit via HomeBridge running on a Raspberry Pi

This was the impetus for creating HomeSpan.




