1. Not case sensitive

2. No support for data section/data types and labelling data.
Only labelling instructions is supported

3. Programmer is responsible for not having duplicate labels
in code (this will be resolved later). If there are duplicate
labels than assembler will still run but references will
be wrong

4. First few instructions of emitted code 
will always perform an initialization
sequence which involves setting up the stack pointer

5. Order of operands in multiply
instruction can impact performance,
put the number you expect to be smaller
as the final operand

6. Possible improvement to decrease
size of programs: have div, srl, mul,
rem, wait instructions as actual subroutines
with predetermined location in
memory that get called when any of these
instructions are executed. Right now
they are expanded inline for every time
one of these pseudoinstructions appears

7. Note: this assembler loads all tokens into
memory to process them. It could conceivably just
load some data into a table rather than loading everything
including the ASCII string of each token into memory.
So performance is not optimal but the assembler 
works fine.

8. There is no predefined calling convention, in test
programs I have written I am dealing with return
values and the return address by shuffling around
values on the stack. Maybe this should be done
with registers? What I have works but isn't
very practical
