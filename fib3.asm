    clear $r0
    clear $r1
    llo $r0, 1
    llo $r1, 1
loop:
    uadd $led, $zero, $r0
    uadd $r2, $r0, $r1
    uadd $r0, $zero, $r1
    uadd $r1, $zero, $r2
    jmp loop
