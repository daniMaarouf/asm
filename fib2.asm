llo  $r0, 1          #f(0) = 1
     llo  $r1, 1          #f(1) = 1
     llo  $r3, 2          #loop counter = 2
     llo  $r4, 1          #N is fibonacci number to compute
     llo  $r5, 1          #for incrementing loop counter
Loop: uadd $r2, $r0, $r1         #f(n) = f(n-1) + f(n-2)
     uadd $r0, $r1, $zero       #shift values for next addition
     uadd $r1, $r2, $zero       #shift values for next addition
     uadd $r3, $r3, $r5         #increment loop counter
     bne  $r3, $r4, Loop        #branch until fib number calculated
     uadd $led, $r2, $zero      #move fib number into LED register