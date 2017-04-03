    load $r0, 33
    rem $r1, $r0, 10
    out $r1             #should be 3
    load $r0, 45
    div $r1, $r0, $r1
    out $r1             #should be 15
    srl $r1             
    mul $r1, $r1, 100
    out $r1             #should be 700

