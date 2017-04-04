1. Not case sensitive

2. Right now only labelling instructions is supported

3. No support for data section/data types and labelling data

4. If there are duplicate labels in code than first one in
file will probably be used

5. First few instructions will always perform an initialization
sequence which involves setting up the stack pointer

12. jmps can take labels
branches can take labels as 3d operand
branches can take literal as 2nd or 3rd operand

arithmetic/logic instructions can take
literal as 3rd operand

14. All numbers treated as unsigned for
MUL, DIV, SRL, REM but dont use numbers
bigger than largest signed

17. when branches are given a literal value
it will be treated as offset

18. note: order of operands in multiply
instruction can greatly impact performance,
put the number you expect to be smaller
as the last operand
