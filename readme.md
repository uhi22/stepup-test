# Step-Up-Converter experiments

## Level 1: A basic small step-up design

![basic schematic of an isolated step up converter](doc/2024-05-05_isolated_stepup_basic_schematic.jpg)

To create the pulses, a simple arduino project is used: https://github.com/uhi22/stepup-test/tree/main/arduino-stepup-test

![on phase, no saturation](doc/2024-05-02_core_EI35_no_saturation.jpg)

![on phase, saturation of the core](doc/2024-05-02_core_EI35_saturation.jpg)
The inductivity L can be determined by the steepness of the current while the voltage is constant: L = U / (dI/dt).

L = 2V / (1.5A / 380µs) = 506µVs/A = 506µH

The stored energy is E = 1/2 * L * I^2.

E_sat = 1/2 * 506µVs/A * 1.5A * 1.5A = 570 µVAs = 0.57mWs


![off phase, avalance of the FET is limiting the voltage](doc/2024-05-02_core_EI35_input_avalance.jpg)

Output with no load
![output unloaded](doc/2024-05-02_core_EI35_output_unloaded.jpg)

Output with D and C
![output charged](doc/2024-05-02_core_EI35_output_charged.jpg)


## Level 2: Increasing the air gap increases the stored energy

Putting a 0.1mm paper stripe between the E and the I of the core.
![saturation with 0.1mm air gap](doc/2024-05-05_core_EI35_with_additional_0.1mm_gap_primary_current.jpg)
The stored energy is E = 1/2 * L * I^2.

The inductivity L can be determined by the steepness of the current while the voltage is constant: L = U / (dI/dt).
L= 2.5V / (4A / 160µs) = 100 µVs/A = 100µH

E_sat = 1/2 * 100µVs/A * 7A * 7A = 2450µVAs = 2.45mWs

Conclusion: Adding 0.1mm air gap to the core increases the storable energy from 0.6mJ to 2.5mJ. In parallel, the inductance decreases by factor 5, which leads to a steeper current ramp, this means that the maximum energy is reached nearly at the same time (around 400µs at ~2.5V supply).

## Level 3: More output voltage

* N1=2, N2=33
* t_on=7µs, t_off=22µs
* R_load=100k
* U_in=4,6V, I_in=1A, P_in=4.6W
* U_out=500V, I_out=5mA, P_out=2.5W

Limiting factors regarding more output power:
- (The 100k load resistor needs to be able to dissipate more power)
- The 7µs on-time nearly leads to de-saturation of the FET due to rising voltage on the shunt. A lower shunt resistance will fix this. This measure will also increase the efficiency.

![2.5W output](doc/2024-05-05_core_EI35_with_additional_0.1mm_gap_output500V_5mA.jpg)

## Level 4: Fixing the shunt and load

* Shunt decreased to 50 mOhm (consisting of 20 x 1 ohm).
* Load resistor 8*12k + 3k9

With this setup we get easily 500V DC, which is 2.5W output without smoke.
The limiting element when increasing the input voltage further, is now the FET, which runs into avalance due the stray inductance, and produces heat until it de-solders. (auto-protecting ;-)

## Level 5: The stray inductance

For N1 we use instead of one now three windings in parallel, distributed over the core, to reduce the stray inductance. With this measure, the resonance frequency in the turn-off phase increases from 16 MHz to 30 MHz.
![stray inductance oscillation with one winding](doc/2024-05-11_stray_inductance_16MHz.jpg)
![stray inductance oscillation with three parallel windings](doc/2024-05-11_stray_inductance_30MHz.jpg)

Nevertheless, the voltage spike at turn-off of the FET nearly reaches the avalance voltage, which
would produce a lot of heat if the voltage is further increased.

How to avoid the large stray-inductance-caused-oscillation?
Some ideas are mentioned here: https://www.joretronik.de/Web_NT_Buch/Kap9/Kapitel9.html

* 10nF + 10 ohms over the primary winding dampens the 30MHz quite well.
* additionally, D + (100nF || 12kohm) flattens the initial spike a little bit. D is a 1N5822.

Result details in Excel. Summary: ~55% efficiency. >4W output reached.

## Level 6: The gate driver

Driving the FET gate directly from the arduino digital output is not ideal, because the arduino has limited
output current capabilities. This leads to slow transitions between the on- and off-phase of the FET,
which causes loss. Faster transitions should lead to less loss.
With arduino output, the transition time (from 5V to 1V) is ~150ns.
When using 7 high-speed-CMOS gates in parallel (74HC08), the transition time goes down to ~60ns.
Surprisingly, the efficiency does not increase. This shows, that the heat is and was produced somewhere else, not in the FET.

Result details in Excel. Summary: The speed of the gate driver is sufficient.

## Level 7: The high voltage measuring

For isolated measurement of the output voltage, we use the "muehlpower board" from here: https://openinverter.org/forum/viewtopic.php?p=41641#p41641
"Based on the isolated amplifier AMC3302DWE from Texas Instruments, it outputs an low impedance analog voltage between 1.42V and 4.8V corresponding to the input voltage of 0V-500V. Isolation is over 2000V, power supply is only required on the low voltage side."

## Level 8: Regulated output voltage

### Which op-amp?
- TL084? No. With 5V supply the common mode voltage range bad (recommented 4V distance to the supply rails).
- LM324? No. Works with single 5V, has 0V input tolerance, but the common-mode range ends at VCC-2V.
- LM358N? Ok. Works with 5V supply. Input common-mode voltage range includes ground until Vcc-1.5V. Large output voltage swing:0V DC to Vcc-1,5V DC.
https://cdn-reichelt.de/documents/datenblatt/A200/358_ESTEK.pdf

### Schematic
- output of the muehlpower board is divided by 5k6 / (5k6+8k2), to not overshoot the common-mode-limit of the op-amp.
- This divider feeds the IN- of the op-amp.
- A poti between 5V and GND feeds the IN+ of the op-amp.
- To get a hysteresis, feedback of 220k from OUT to IN+.
- The OUT of the op-amp goes to the AND in the 74HC08. The other side of the AND is driven by the Arduino PWM.
- The output of the AND goes to the 7 74HC08 gates in parallel, which are the gate driver.

Results: good regulation. Details in Excel.

## Level 9: Software-adjustable output voltage

To control the output voltage from PC, the core element is an ESP32 microcontroller, more precise a WT32-ETH01 board. This provides an I2C bus
where we have an D-A-converter (MCP4725, 12 bits) and an A-D-converter (ADS1015, 4 channel, 12 bits).
The DAC provides 0 to 3.3V. This feeds via 2k7 the IN+ of the op-amp.
The ADC is connected to the divided muehlpower voltage, this means it sees the same voltage as the IN- of the op-amp.
The ethernet port of the WT32-ETH01 connects to a homeplug modem, which is configured as PEV, so that it is able to receive
the SLAC messages which are sent by an EVSE controller. Re-using some SLAC messages has the advantage, that no pairing
of the PLC modems is necessary.

On PC, the pyPLC (https://github.com/uhi22/pyPLC) is used in EvseMode to communicate with the WT32-ETH01 via PLC. It can set the target voltage and receive the present voltage.
The software on the WT32-ETH01 is an arduino project, https://github.com/uhi22/stepup-test/tree/main/arduino-wt32eth.

Results: pyPLC controls the physical voltage during the precharge phase of a CCS charging session. pyPLC displays the measured voltage.

## Level 10: Overcurrent protection


## Level 11: Current Path Optimization

The relatively thin wires, together with multiple centimeters length, create a certain loss. The following measures should improve this:
* Use thicker wires for the primary winding, and more in parallel.
* Use thicker wire for the secondary winding.
* Make the lines between transformer, transistor, shunt and capacitors as short as possible and thick.

## References

https://www.joretronik.de/Web_NT_Buch/Kap7_2/Kapitel7_2.html
