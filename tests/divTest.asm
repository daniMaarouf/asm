    load $r0 0b00001110
    srl $r1 $r0
    out $r1
    srl $r1 0b00001111
    out $r1
    load $r0 45
    div $r1, $r0, 10
    out $r1
    load $r2 19
    div $r3, $r2, $r1
    out $r3
    sll $r3, $r3
    out $r3             #should be 8
