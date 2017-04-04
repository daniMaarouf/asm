    load $r0 3
    out $r0
    load $r1 4
    out $r1
    load $r2 1
    out $r2

    mul $r3, $r2, $r0
    out $r3
    out $r2
    mul $r2, $r2, $r0
    out $r2
    mul $r2, $r0, 4
    out $r2
