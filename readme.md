# Step-Up-Converter experiments

## Level 1: A basic small step-up design

![basic schematic of an isolated step up converter](doc/2024-05-05_isolated_stepup_basic_schematic.jpg)

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

## References

https://www.joretronik.de/Web_NT_Buch/Kap7_2/Kapitel7_2.html
