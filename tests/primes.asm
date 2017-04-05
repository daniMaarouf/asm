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
    out $r3
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
