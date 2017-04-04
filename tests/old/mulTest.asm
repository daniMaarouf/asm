    load $r0 15
    load $r1 10
    mul $r2, $r1, $r0
    div $r3, $r2, 75
    load $r0 8
    mul $r4, $r0, $r3
    out $r4             #should be 16
