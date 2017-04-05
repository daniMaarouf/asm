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

