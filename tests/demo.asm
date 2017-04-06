input:    
    out 0x1111              #this means program expects you to chose demo
    in $r0
    and $r1, $r0, 0x0080
    beq $r1, $zero, input   #wait until user flips 8th switch
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
    bne $r1, 0x2, notExponentiation
    call exponentiation
    jmp end
notExponentiation:
    bne $r1, 0x3, notPrimeList
    call primeList
    jmp end
notPrimeList:
    bne $r1, 0x4, notPrimeFactors
    call primeFactors
    jmp end
notPrimeFactors:
    bne $r1, 0x5, notExtra1
    call extra1
    jmp end
notExtra1:
    bne $r1, 0x6, notExtra2
    call extra2
    jmp end
notExtra2:
    out 0xFFFF          #this status code means that no demo was chosen
    wait 2000
end:
    in $r0
    and $r1, $r0, 0x0080
    out 0xEEEE          #this means pull down upper switch
    bne $r1, $zero, end
    out 0x0000          #default
    jmp input

extra1:
    out 0xAAAA
    wait 200
    ret

extra2:
    out 0xBBBB
    wait 200
    ret

basicFib:
    load $r0, 1
    load $r1, 1
basicFibInput:
    out 0x3333
    in $r3
    and $r4, $r3, 0x80
    bne $r4, $zero, basicFibInput
    and $r3, $r3, 0x7F
    load $r2 1
    load $r4 1
basicFibLoop:
    bge $r4, $r3, basicFibEnd
    uadd $r2, $r0, $r1
    load $r0, $r1
    load $r1, $r2
    inc $r4
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
    load $r2 1
recursiveFibLoop:
    bgt $r1, $r0, recursiveFibEnd
    
    push $r1
    call calcFib
    call calculateBCD
    pop $r2

    out $r2
    inc $r1
    wait 250
    jmp recursiveFibLoop
recursiveFibEnd:
    out $r2
    wait 3000
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
    out 0x5555
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, exponentiation
expLoop1:
    out 0x6666
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
    out 0x7777
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
    out 0x8888
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, primeFactors
factorsGetHiBits:
    out 0x9999
    in $r1
    and $r2, $r1, 0x80
    beq $r2, $zero, factorsGetHiBits
    
    and $r1, $r1, 0x7F

    clear $r2

factorsShiftHiBits:
    sll $r1, $r1
    inc $r2
    bne $r2, 7, factorsShiftHiBits

    or $r0, $r0, $r1

    bgt $r0, 1, factorsCheckPrime
    out 0xFFFF
    wait 1000
    ret

factorsCheckPrime:
    push $r0
    call isPrime
    pop $r1
    beq $r1, $zero, factorsNotPrime
    out 0xFFFF
    wait 200
    push $r0
    call calculateBCD
    pop $r0
    out $r0
    wait 500
    ret
factorsNotPrime:
    clear $r1
    clear $r3
primeFactorsLoop:    
    beq $r1, $r0, primeFactorsComplete
    push $r1
    call isPrime
    pop $r2
    inc $r1
    beq $r2, 0, primeFactorsLoop
    uadd $r2, $r1, -1
    sw $r2, 0($r3)
    inc $r3
    jmp primeFactorsLoop
primeFactorsComplete:

    clear $r1
    clear $r2
    clear $r4
    clear $r5
    load $r6 0x400

primeFactorsCalc:
    beq $r1, $r3, primeFactorsPrint
    lw $r2, 0($r1)
    rem $r5, $r0, $r2
    inc $r1
    bne $r5, 0, primeFactorsCalc
    sw $r2, $r6
    inc $r6
    div $r0, $r0, $r2
    clear $r1
    jmp primeFactorsCalc
primeFactorsPrint:
    load $r1, $r6
    load $r0, 0x400
factorsPrintLoop:
    beq $r0, $r1, factorsRet
    lw $r2, $r0
    inc $r0
    push $r2
    call calculateBCD
    pop $r2
    out $r2
    wait 500
    out 0
    wait 200
    jmp factorsPrintLoop
factorsRet:
    wait 1000
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

