# LC3-Simulator
Lab3 in CS169, Shanghai Jiao Tong University, 2020 Fall.

Run it in Linux and std99.
The input file should be .hex files consisting of 4 hex characters per line.

>>gcc -std=c99 -o simulate main.c
>>./simulate <main_program_file> [extra_file] [extra_file] ...

1.go: simulate the program until a HALT instruction is executed.
2. run <n>: simulate the execution of the machine for n instructions.
3. mdump <low> <high>: dump the contents of memory, from location low to location high to the screen and file.
4. rdump: dump the current instruction count, the contents of R0–R7, PC, and condition codes to the screen and file.
5. ?: print out a list of all shell commands.
6. quit: quit the shell
