     out 0
     wait 1000
     out 1
     wait 1000
     out 2
     wait 10000
     out 3
end: out 1
     beq $zero, $zero, end
     out 3
