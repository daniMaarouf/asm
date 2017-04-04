    load $r0 47
    div $r1, $r0, 10
    out $r1         #output should be 4
    load $r0 49
    div $r1, $r0, 10
    out $r1         #output should be 4
    load $r0 50
    div $r1, $r0, 10
    out $r1         #output should be 5
    load $r0 0xFE
    div $r1, $r0, 2
    out $r1         #output should be 0x7F

