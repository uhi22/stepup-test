
# Which messages are passing the homeplug modem in un-paired state?

## AR7420, configured as PEV

* CM_SLAC_PARAM.CNF (0x6065) In this message, at least the RunID (8 bytes) and additional 16 padding bytes in the end could be used for special features.
* CM_ATTEN_CHAR.IND (0x606e) Lot of space, e.g. the attenuation group list, can be re-used. This works also if the destination MAC is broadcast.
* CM_SLAC_MATCH.CNF (0x607d) not recommended to re-use, to avoid side effects.

