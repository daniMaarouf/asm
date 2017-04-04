        load $r0 3
        load $r1 4
        load $r2 1
        
loop:   beq $r1, $zero, end
        mul $r2, $r2, $r0
        out $r2
        dec $r1
        jmp loop
end:    out $r2
        jmp end
