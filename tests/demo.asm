input:    
    out 0x1111          #this means program expected you to chose demo
    in $r0
    and $r1, $r0, 0x0080
    beq $r1, $zero, input #wait until user flips 8th switch
    out 0x0000

    and $r1, $r0, 0x007F
    
    bne $r1, 0x0, notBasicFib
    call basicFib
    jmp end
notBasicFib:
    bne $r1, 0x1, notRecursiveFib
    call recursiveFib
    jmp end
notRecursiveFib:
    bne $r2, 0x2, notExponentiation
    call exponentiation
    jmp end
notExponentiation:
    bne $r2, 0x3, notPrimeList
    call primeList
    jmp end
notPrimeList:
    bne $r2, 0x4, notPrimeFactors
    call primeFactors
    jmp end
notPrimeFactors:
    out 0xFFFF      #this status code means that no demo was chosen
    wait 1000
end:
    in $r0
    and $r1, $r0, 0x0080
    out 0x2222      #this means pull down upper switch
    bne $r1, $zero, end
    out 0x0000      #default
    jmp input

basicFib:
    load $r0, 1
    load $r1, 1
basicFibInput:
    out 0x3333
    in $r3
    and $r4, $r3, 0x80
    bne $r4, $zero, basicFibInput
    and $r3, $r3, 0x7F
    clear $r2
    clear $r4
basicFibLoop:
    beq $r4, $r3, basicFibEnd
    uadd $r2, $r0, $r1
    load $r0, $r1
    load $r1, $r2
    jmp basicFibLoop
basicFibEnd:
    push $r2
    call calculateBCD
    pop $r2
    out $r2
    wait 1000
    ret

recursiveFib:
    out 0x4444
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, recursiveFib
    and $r0, $r0, 0x7F
    load $r1 0
recursiveFibLoop:
    bge $r1, $r0, recursiveFibEnd
    push $r1
    call calcFib
    pop $r2
    
    push $r2
    call calculateBCD
    pop $r2

    out $r2
    inc $r1
    wait 250
    jmp recursiveFibLoop
recursiveFibEnd:
    out $r2
    wait 1000
    ret

calcFib:
    pop $r7     #return address now in r7
    pop $r4     #argument now in $r4
    bgt $r4, 1, recurse
    push 1      #return value is 1
    push $r7
    ret
recurse:
    push $r7    #put return address back on stack
    usub $r4, $r4, 1
    push $r4            #first number to calc
    usub $r4, $r4, 1
    push $r4            #second number to calc
    call calcFib
    pop $r4     #pop new return value
    pop $r5
    push $r4    #stash the new return value for first
    push $r5    #put second number to calc on top of stack
    call calcFib
    pop $r4     #second return val
    pop $r5     #first return val
    pop $r7     #return address
    uadd $r4, $r4, $r5
    push $r4
    push $r7
    ret

exponentiation:
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, exponentiation
expLoop1:
    in $r1
    and $r2, $r1, 0x80
    beq $r2, $zero, expLoop1
    and $r0, $r0, 0x7F
    and $r1, $r1, 0x7F
    load $r2 1
expLoop2:
    beq $r1, $zero, expEnd
    mul $r2, $r2, $r0
    dec $r1
    jmp expLoop2
expEnd:
    push $r2
    call calculateBCD
    pop $r2
    out $r2
    wait 1000
    ret

primeList:
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, primeList
    clear $r1

primeListLoop:    
    beq $r1, $r0, primeListEnd
    push $r1
    call isPrime
    pop $r2
    inc $r1
    beq $r2, 0, primeListLoop
    uadd $r3, $r1, -1
    push $r3
    call calculateBCD
    pop $r3
    out $r3
    wait 200
    jmp primeListLoop
primeListEnd:
    wait 1000
    ret

primeFactors:
    call primeList
    wait 5000
    ret

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

