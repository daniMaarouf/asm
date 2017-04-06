1. Nothing is case sensitive

2. No support for data section/data types and labelling data.
Only labelling instructions is supported

3. Programmer is responsible for not having duplicate labels
in code (this will be resolved later). If there are duplicate
labels than references will probably point to the first one
which appears in the file

4. First few instructions of emitted code 
will always perform an initialization
sequence which involves setting up the stack pointer

5. For MUL, DIV, SRL, REM dont use negative
numbers or numbers bigger 0x7FFF

7. If branch instruction given an integer literal
as 3rd argument is will be treated as offset with
respect to the first instruction after the branch.
It is not recommended you use this feature as you
probably don't know how many primitive instructions
each pseudoinstruction actually takes.

8. Order of operands in multiply
instruction can impact performance,
put the number you expect to be smaller
as the final operand

9. Possible improvement to decrease
size of programs: have div, srl, mul,
rem, wait instructions as actual subroutines
with predetermined location in
memory that get called when any of these
instructions are executed. Right now
they are expanded inline for every time
one of these pseudoinstructions appears

10. Note: this assembler loads all tokens into
memory to process them. It could conceivably just
load some data into a table rather than loading everything
including the ASCII string of each token into memory.
So performance is not optimal but the assembler 
works fine.

13. There is no predefined calling convention, in test
programs I have written I am dealing with return
values and the return address by shuffling around
values on the stack. Maybe this should be done
with registers? What I have works but isn't
very practical

14. Don't have multiple comments in row, just at
end of line.
