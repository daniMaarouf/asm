1. Not case sensitive

2. Right now only labelling instructions is supported

3. No support for data section/data types and labelling data

4. If there are duplicate labels in code than first one in
file will probably be used. Add duplicate check later

5. First few instructions will always perform an initialization
sequence which involves setting up the stack pointer

6. jmps can take labels
branches can take labels as 3d operand
branches can take literal as 2nd or 3rd operand

arithmetic/logic instructions can take
literal as 3rd operand

7. All numbers treated as unsigned for
MUL, DIV, SRL, REM but dont use numbers
bigger than largest signed

8. when branches are given a literal value
it will be treated as offset

9. note: order of operands in multiply
instruction can greatly impact performance,
put the number you expect to be smaller
as the last operand

10. Possible improvement to decrease
length of programs: have div, srl, mul,
rem, wait instructions as actual subroutines
with predetermined location in
memory that get called when any of these
instructions are executed.

11. Note: this assembler loads all tokens into
memory to process them. Maybe this is not optimal/
maybe it could use less memory. Despite this the
assembler seems to work fine.

12. LW and SW offsets still need to be tested

13. Not predefined calling convention, in test
programs I have written I am dealing with return
values and the return address by shuffling around
values on the stack. Maybe this should be done
with registers? What I have works but isn't
very practical
