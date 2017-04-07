#ENGG3380 Term Project - Phase 3
#Code for demo
#Written by Dani Maarouf

input:    
    out 0x1111              #program wants you to choose demo
    in $r0
    and $r1, $r0, 0x0080
    beq $r1, $zero, input   #wait until user flips 8th switch
    
    and $r1, $r0, 0x007F    #8th bit not relevant
    
    bne $r1, 0x0, notBasicFib   #switch statement for desired function
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
    bne $r1, 0x5, notClassDemo
    call classDemo
    jmp end
notClassDemo:
    out 0xFFFF          #no program chosen, user input couldn't be matched
    wait 2000
end:
    in $r0
    and $r1, $r0, 0x0080
    out 0xEEEE          #make user pull down 8th switch before looping back
    bne $r1, $zero, end
    jmp input

#template subroutine which can be filled in on demo day
classDemo:
    out 0x9999          #get user input
    in $r3
    and $r4, $r3, 0x80
    bne $r4, $zero, classDemo
    
    and $r3, $r3, 0x7F      #8th bit of user input not relevant
    load $r0, 1             #f(0) = f(1) = 1
    sw $r0, 0               #store f(0) = 1in memory location 0
    load $r0, 2
    sw $r0, 1               #store f(1) = 2in memory location 1
    load $r4 2              #loop index
classDemoLoop:
    bgt $r4, $r3, classDemoEnd   #desired fib has been calculated and is in $r2
    lw $r0, -2($r4)
    sll $r0, $r0
    lw $r1, -1($r4)
    uadd $r2, $r0, $r1
    sw $r2, 0($r4)
    inc $r4                     #increment loop index
    jmp classDemoLoop
classDemoEnd:
    lw $r0, 0($r3)              #get the fib value
    push $r0                    #call with argument
    call calculateBCD
    pop $r0                     #get return value
    out $r0
    wait 1000                   #stall for 1000ms
    ret

#template subroutine which can be filled in on demo day
extra2:
    out 0xBBBB          
    wait 200
    ret

#subroutine which simply prints out fib number that
#user choses with switches
basicFib:
    out 0x2222          #get user input
    in $r3
    and $r4, $r3, 0x80
    bne $r4, $zero, basicFib
    
    and $r3, $r3, 0x7F      #8th bit of user input not relevant
    load $r0, 1             #f(0) = f(1) = 1
    sw $r0, 0               #store f(0) in memory location 0
    sw $r0, 1               #store f(1) in memory location 1
    load $r4 2              #loop index
basicFibLoop:
    bgt $r4, $r3, basicFibEnd   #desired fib has been calculated and is in $r2
    lw $r0, -2($r4)
    lw $r1, -1($r4)
    uadd $r2, $r0, $r1
    sw $r2, 0($r4)
    inc $r4                     #increment loop index
    jmp basicFibLoop
basicFibEnd:
    lw $r0, 0($r3)              #get the fib value
    push $r0                    #call with argument
    call calculateBCD
    pop $r0                     #get return value
    out $r0
    wait 1000                   #stall for 1000ms
    ret

#subroutine which prints out the fibonacci numbers up to
#and including the number specified by user
recursiveFib:
    out 0x3333                  #get user input
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, recursiveFib
    and $r0, $r0, 0x7F
    load $r1 0
    load $r2 1
recursiveFibLoop:
    bgt $r1, $r0, recursiveFibEnd   #done calculating number
    
    push $r1               #we want fib($r1)
    call calcFib           #return value of calcFib becomes argument of calculateBCD
    call calculateBCD
    pop $r2

    out $r2                 #print fibs as they are calculated
    inc $r1
    wait 250                #250ms
    jmp recursiveFibLoop
recursiveFibEnd:
    out $r2                 #print final value
    wait 3000               #wait 3s
    ret

#subroutine which is recursive, calculates fib(n)
#for the n which is passed as argument through the stack
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

#subroutine, calculates the result when base that user
#provides is exponentiated with exponent that user provides
exponentiation:
    out 0x4444                      #get first number
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, exponentiation
expLoop1:
    out 0x5555                      #get second number
    in $r1
    and $r2, $r1, 0x80
    beq $r2, $zero, expLoop1
    and $r0, $r0, 0x7F
    and $r1, $r1, 0x7F
    load $r2 1                      #start with 1
expLoop2:
    beq $r1, $zero, expEnd
    mul $r2, $r2, $r0               #repeatedly multiply
    dec $r1
    jmp expLoop2
expEnd:
    push $r2
    call calculateBCD
    pop $r2
    out $r2                         #print BCD representation
    wait 1000
    ret

#subroutine, prints prime numbers up to but not including
#the value that the user gives with switches
primeList:
    out 0x6666                      #user input
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, primeList
    clear $r1

primeListLoop:    
    beq $r1, $r0, primeListEnd      #done
    push $r1
    call isPrime
    pop $r2
    inc $r1
    beq $r2, 0, primeListLoop       #if not prime keep looping
    uadd $r3, $r1, -1               #prime, deincrement
    push $r3 
    call calculateBCD
    pop $r3 
    out $r3                         #print BCD representation
    wait 200
    jmp primeListLoop
primeListEnd:
    wait 1000
    ret

#subroutine, prints prime factorization for an integer
#which user provides through switch input
primeFactors:
    out 0x7777                      #get lower 7 bits
    in $r0
    and $r1, $r0, 0x80
    bne $r1, $zero, primeFactors
factorsGetHiBits:
    out 0x8888                      #get upper 7 bits
    in $r1
    and $r2, $r1, 0x80
    beq $r2, $zero, factorsGetHiBits
    
    and $r1, $r1, 0x7F

    clear $r2

factorsShiftHiBits:                 
    sll $r1, $r1                    #shift upper 7 bits left 7 times
    inc $r2
    bne $r2, 7, factorsShiftHiBits

    or $r0, $r0, $r1                #or upper and lower bits to get full 14 bit num

    bgt $r0, 1, factorsCheckPrime   #if number is less than 2 than no prime factorization
    out 0xFFFF
    wait 1000
    ret

factorsCheckPrime:
    push $r0                        #if the number is prime than theres no factorization to be done
    call isPrime
    pop $r1
    beq $r1, $zero, factorsNotPrime
    out 0xDDDD                      #if prime print status message, print out the prime and return
    wait 500
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
    beq $r1, $r0, primeFactorsComplete      #calculate all the primes less than user given number
    push $r1
    call isPrime
    pop $r2
    inc $r1
    beq $r2, 0, primeFactorsLoop
    uadd $r2, $r1, -1
    sw $r2, 0($r3)                          #store the calculated primes starting at 0x0 in memory
    inc $r3                                 #r3 stores highest prime
    jmp primeFactorsLoop
primeFactorsComplete:
    clear $r1                               #new loop index for divide loop
    clear $r2
    clear $r4
    clear $r5
    load $r6 0x400                          #store prime factors starting at 0x400 (arbitrary address)
primeFactorsCalc:
    beq $r1, $r3, primeFactorsPrint         #once new loop index reaches final prime calculated: exit
    lw $r2, 0($r1)                          #load the appropriate prime
    rem $r5, $r0, $r2                       #see if it divides our number with no remainder
    inc $r1
    bne $r5, 0, primeFactorsCalc
    sw $r2, $r6                             #store the prime factor
    inc $r6
    div $r0, $r0, $r2                       #if the prime divides number with no remainder than divide our number
    clear $r1                               #set loop index back to 0 to continue factorization
    jmp primeFactorsCalc
primeFactorsPrint:
    load $r1, $r6                           #final memory location of prime factors in $r6
    load $r0, 0x400                         #loop index starting at 0x400
factorsPrintLoop:
    beq $r0, $r1, factorsRet                #when loop index reaches last prime factor: break
    lw $r2, $r0
    inc $r0
    push $r2
    call calculateBCD
    pop $r2
    out $r2
    wait 750
    out 0
    wait 300
    jmp factorsPrintLoop
factorsRet:
    wait 200
    ret
   
#subroutine, returns boolean value on stack which
#indicates whether or not argument is prime number
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

#subroutine, returns BCD representation
#of argument on stack
calculateBCD:
    pop $r7
    pop $r4
    push $r7
    clear $r6

    rem $r5, $r4, 10        #extract first decimal digit
    or $r6, $r6, $r5
    
    div $r5, $r4, 10        #extract second decimal digit
    rem $r5, $r5, 10
    clear $r7
loop1:
    sll $r5, $r5            #shift decimal digit left by 4
    inc $r7
    bne $r7, 4, loop1
    or $r6, $r6, $r5        #or the BCD encoded digit to final result
    
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

    pop $r7                 #pop return address
    push $r6                #push return value
    push $r7                #push return address
    ret

