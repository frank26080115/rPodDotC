this code is meant to replace rI2CRX and rI2CTX

improvements over rI2CRX/TX:

supports multiple instances
cleaner sync
ability to redundantly retransmit to overcome physical transport issues
much stronger checksum that is position dependant (see fletcher.c)
better suited for UART due to the way FIFOs work (UART is not automatically packetized, there's no start or stop like I2C, David's code made sense for I2C because of those)

cons:

no packet builder, but I did provide a generic parameter read/write structure that works in a similar way, but the user has to track an index