    load $r0 0
loop:
    bge $r0, 10, end
    push $r0        #register 0 passed as argument
    call calcFib
    pop $r1         #register 1 stores return value
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

