    load $r0 0
loop:
    bge $r0, 20, end
    push $r0        #register 0 passed as argument
    call calcFib
    pop $r1         #register 1 stores return value
    
    push $r1
    call calculateBCD
    pop $r1

    out $r1
    inc $r0         #increment index
    jmp loop
end:
    out $r1
    jmp end

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

