    load $r0 0
    load $r1 1
    load $r2 0
    load $r3 0
loop: 
    beq $r3, 10, end
    call calcDec
    out $r0
    wait 500
    uadd $r2, $r0, $r1
    load $r0 $r1
    load $r1 $r2
    inc $r3
    jmp loop
end:
    out $r2
    jmp end

calcDec:
    ret

