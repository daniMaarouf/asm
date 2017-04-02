        llo $r0, 1      #test comment
        llo $r1, 1
Loop2:
        Loop1:   uadd $r2, $r1, $r0
        uadd $r0, $r1, $zero
        uadd $r1, $r2, $zero
        llo $r6, 0xFF
        lhi $r6, 0b01111111
        sw $r6, $r0
        lw $led, 0($r6)
        llo $r7, 2
        lhi $r7, 0
        jmp $r7

