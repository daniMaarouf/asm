    load $r0, 100
    clear $r1
mainLoop:    
    beq $r1, $r0, mainEnd
    push $r1
    call isPrime
    pop $r2
    inc $r1
    beq $r2, 0, mainLoop
    uadd $r3, $r1, -1
    push $r3
    call calculateBCD
    pop $r3
    out $r3
    wait 200
    jmp mainLoop
mainEnd:
    beq $zero, $zero, mainEnd

isPrime:
    pop $r7
    pop $r4
    bgt $r4, 1, cont1
    push 0
    push $r7
    ret
cont1:
    bgt $r4, 3, cont2
    push 1
    push $r7
    ret
cont2:
    load $r5 2
primeLoop:
    usub $r6, $r4, 1
    bgt $r5, $r6, primeEnd
    rem $r6, $r4, $r5
    inc $r5
    bne $r6, 0, primeLoop
    push 0
    push $r7
    ret
primeEnd:
    push 1
    push $r7
    ret

calculateBCD:
    pop $r7
    pop $r4
    push $r7
    
    clear $r6
    
    rem $r5, $r4, 10
    or $r6, $r6, $r5
    
    div $r5, $r4, 10
    rem $r5, $r5, 10
    clear $r7
loop1:
    sll $r5, $r5
    inc $r7
    bne $r7, 4, loop1
    or $r6, $r6, $r5
    
    div $r5, $r4, 100
    rem $r5, $r5, 10
    clear $r7
loop2:
    sll $r5, $r5
    inc $r7
    bne $r7, 8, loop2
    or $r6, $r6, $r5

    div $r5, $r4, 1000
    rem $r5, $r5, 10
    clear $r7
loop3:
    sll $r5, $r5
    inc $r7
    bne $r7, 12, loop3
    or $r6, $r6, $r5

    pop $r7
    push $r6
    push $r7
    ret

