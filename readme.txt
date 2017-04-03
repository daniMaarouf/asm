1. Not case sensitive

2. Right now only labelling instructions is supported

3. No support for data section/data types and labelling data

4. If there are duplicate labels in code than first one in
file will probably be used

5. First instruction should load the stack pointer

6. rewrite srl, div, mul, rem without bugs

7. next stage after counting: addresses and label resolution

8. Handle negative immediate values

9. Check push instruction again

10. order of sw and lw operands

11. Note: srl and division work but 
are EXTREMELY slow. expect them to take
1000x-10000x+ a regular instruction

12. jmps can take labels
branches can take labels as 3d operand
branches can take literal as 2nd or 3rd operand

arithmetic/logic instructions can take
literal as 3rd operand

13. add call and ret

