    load $r0 0
    load $r1 1
    load $r2 0
    load $r3 0
loop: 
    beq $r3, 10, end1
    push $r0
    call calculateBCD
    pop $r0
    out $r0
    wait 500
    uadd $r2, $r0, $r1
    load $r0 $r1
    load $r1 $r2
    inc $r3
    jmp loop
end1:
    out $r2
    jmp end

calculateBCD:
    pop $r7         #return address
    pop $r4         #argument
    

end2:
    push $r4        #return value
    push $r7        #return address
    ret

