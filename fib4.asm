    llo $r0 0b01010101
    lhi $r0 0x2
    uadd $led, $r0, $zero
    srl $r0
    uadd $led, $r0, $zero
