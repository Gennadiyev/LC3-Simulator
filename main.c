#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3 machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/*
  MEMORY[A] stores the word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3 State info.                                           */
/***************************************************************/
#define LC_3_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

    int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
    int REGS[LC_3_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3 ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3 for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3 until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = start ; address <= stop ; address++)
        printf("  0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = start ; address <= stop ; address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;

        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;

        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;

        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word ;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii] = word;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = program_base;

    printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3 Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}
/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

#define Low16bits(x) ((x) & 0xFFFF)

int Instruction;

/* Stack */
/*
 * 0x4000 -- 0x3FFB
 * Stack to save R7 automatically
 * top_p is the pointer to the top of the stack
 * pop to get the top of stack
 * push to put latch into stack
 */
int top_p = 0x4000;

int IsEmpty(){
    return top_p == 0x4000;
}

int POP(){
    top_p += 1;
    return MEMORY[top_p - 1];
}

int PUSH(int R7){
    top_p -= 1;
    MEMORY[top_p] = R7;
    return 0;
}

/* End of stack */

int SetCC(int Result);  /* Update condition code */
int SEXT(int num, int length);  /* Sign extension */
int ADD(int Inst);  /* 0001 *//* SetCC */
int AND(int Inst);  /* 0101 *//* SetCC */
int BR(int Inst);   /* 0000 */
int JMP(int Inst);  /* 1100 */
int JSR(int Inst);  /* 0100 */
int JSRR(int Inst); /* 0100 */
int LD(int Inst);   /* 0010 *//* SetCC */
int LDI(int Inst);  /* 1010 *//* SetCC */
int LDR(int Inst);  /* 0110 *//* SetCC */
int LEA(int Inst);  /* 1110 */
int NOT(int Inst);  /* 1001 *//* SetCC */
int ST(int Inst);   /* 0011 */
int STI(int Inst);  /* 1011 */
int STR(int Inst);  /* 0111 */
int TRAP(); /* 1111 *//* Halt */

void process_instruction(){
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */

    /* Fetch */
    Instruction = MEMORY[CURRENT_LATCHES.PC];
    CURRENT_LATCHES.PC += 1;
    Instruction = Low16bits(Instruction);

    /* Decode */
    int Op = Instruction & 0xF000;  /* Instruction[15:12] */

    /* Execute */
    switch (Op) {
        case 0x1000:    /* 0001 */
            ADD(Instruction);
            break;

        case 0x5000:    /* 0101 */
            AND(Instruction);
            break;

        case 0x0000:    /* 0000 */
            BR(Instruction);
            break;

        case 0xC000:    /* 1100 */
            JMP(Instruction);
            break;

        case 0x4000:    /* 0100 */
            if((Instruction & 0x0800) >> 11)    /* Instruction[11] = 1 */
                JSR(Instruction);
            else    /* Instruction[11] = 0 */
                JSRR(Instruction);
            break;

        case 0x2000:    /* 0010 */
            LD(Instruction);
            break;

        case 0xA000:    /* 1010 */
            LDI(Instruction);
            break;

        case 0x6000:    /* 0110 */
            LDR(Instruction);
            break;

        case 0xE000:    /* 1110 */
            LEA(Instruction);
            break;

        case 0x9000:    /* 1001 */
            NOT(Instruction);
            break;

        case 0x3000:    /* 0011 */
            ST(Instruction);
            break;

        case 0xB000:    /* 1011 */
            STI(Instruction);
            break;

        case 0x7000:    /* 0111 */
            STR(Instruction);
            break;

        case 0xF000:    /* 1111 */
            TRAP();
            break;

        default:
            break;
    }

    /* Update NEXT_LATCHES */
    NEXT_LATCHES = CURRENT_LATCHES;
}


int SetCC(int Result){      /* Set the condition code */

    if(Result == 0){    /* Zero */
        CURRENT_LATCHES.N = 0;
        CURRENT_LATCHES.Z = 1;
        CURRENT_LATCHES.P = 0;
        return 0;
    }

    int Sign = Low16bits(Result) & 0x8000;  /* Sign bit */

    if(Sign == 0){  /* Positive */
        CURRENT_LATCHES.N = 0;
        CURRENT_LATCHES.Z = 0;
        CURRENT_LATCHES.P = 1;
        return 0;
    }

    if(Sign > 0){   /* Negative */
        CURRENT_LATCHES.N = 1;
        CURRENT_LATCHES.Z = 0;
        CURRENT_LATCHES.P = 0;
        return 0;
    }
}

int SEXT(int num, int length){  /* Sign extension */
    int Sign;
    switch (length) {
        case 5:     /* imm5 */
            Sign = (num & 0x0010) >> 4;
            if(Sign)        /* Negative */
                num += 0xFFE0;  /* Extend with 1 */
            break;

        case 6:     /* PCoffset6 */
            Sign = (num & 0x0020) >> 5;
            if(Sign)
                num += 0xFFC0;
            break;

        case 9:     /* PCoffset9 */
            Sign = (num & 0x0100) >> 8;
            if(Sign)
                num += 0xFE00;
            break;

        case 11:    /* PCoffset11 */
            Sign = (num & 0x0400) >> 10;
            if(Sign)
                num += 0xF800;
            break;

        default:
            break;
    }
    num = Low16bits(num);
    return num;
}

int ADD(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int SR1 = (Inst & 0x01C0) >> 6;

    if((Inst & 0x0020) == 0) {      /* Instruction[5] = 0 */
        int SR2 = (Inst & 0x0007);
        CURRENT_LATCHES.REGS[DR]
                = CURRENT_LATCHES.REGS[SR1] + CURRENT_LATCHES.REGS[SR2];
    }

    else{      /* Instruction[5] = 1 */
        int imm5 = (Inst & 0x001F);
        CURRENT_LATCHES.REGS[DR] = CURRENT_LATCHES.REGS[SR1] + SEXT(imm5, 5);
    }

    CURRENT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[DR]);
    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}

int AND(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int SR1 = (Inst & 0x01C0) >> 6;

    if((Inst & 0x0020) == 0) {            /* Instruction[5] = 0 */
        int SR2 = (Inst & 0x0007);
        CURRENT_LATCHES.REGS[DR] = CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2];
    }

    else{       /* Instruction[5] = 1 */
        int imm5 = (Inst & 0x001F);
        CURRENT_LATCHES.REGS[DR] = CURRENT_LATCHES.REGS[SR1] & SEXT(imm5, 5);
    }

    CURRENT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[DR]);
    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}

int BR(int Inst){
    int PCoffset9 = Inst & 0x01FF;
    int n = (Inst & 0x0800) >> 11;
    int z = (Inst & 0x0400) >> 10;
    int p = (Inst & 0x0200) >> 9;

    int flag = n * CURRENT_LATCHES.N + z * CURRENT_LATCHES.Z + p * CURRENT_LATCHES.P;  /* Condition */

    if(flag){
        CURRENT_LATCHES.PC += SEXT(PCoffset9, 9);
        CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC);
    }

    return 0;
}

int JMP(int Inst){
    int BaseR = (Inst & 0x01C0) >> 6;
    if(BaseR == 7 && !IsEmpty())        /* RET */
        CURRENT_LATCHES.REGS[7] = POP();

    CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
    return 0;
}

int JSR(int Inst){
    CURRENT_LATCHES.REGS[7] = CURRENT_LATCHES.PC;  /* Save R7 first */
    PUSH(CURRENT_LATCHES.REGS[7]);

    int PCoffset11 = Inst & 0x07FF;
    CURRENT_LATCHES.PC += SEXT(PCoffset11, 11);
    CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC);
    return 0;
}

int JSRR(int Inst){
    CURRENT_LATCHES.REGS[7] = CURRENT_LATCHES.PC;   /* Save R7 first */
    PUSH(CURRENT_LATCHES.REGS[7]);

    int BaseR = (Inst & 0x01C0) >> 6;
    CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
    return 0;
}

int LD(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int PCoffset9 = Inst & 0x01FF;

    int Addr = Low16bits((CURRENT_LATCHES.PC + SEXT(PCoffset9, 9)));
    CURRENT_LATCHES.REGS[DR] = Low16bits(MEMORY[Addr]);

    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}

int LDI(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int PCoffset9 = Inst & 0x01FF;

    int Addr = Low16bits((CURRENT_LATCHES.PC + SEXT(PCoffset9, 9)));
    CURRENT_LATCHES.REGS[DR] = Low16bits(MEMORY[MEMORY[Addr]]);

    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}

int LDR(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int BaseR = (Inst & 0x01C0) >> 6;
    int PCoffset6 = Inst & 0x003F;

    int Addr = Low16bits((CURRENT_LATCHES.REGS[BaseR] + SEXT(PCoffset6, 6)));
    CURRENT_LATCHES.REGS[DR] = Low16bits(MEMORY[Addr]);

    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}


int LEA(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int PCoffset9 = Inst & 0x01FF;
    CURRENT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.PC + SEXT(PCoffset9, 9));
    return 0;
}

int NOT(int Inst){
    int DR = (Inst & 0x0E00) >> 9;
    int SR = (Inst & 0x01C0) >> 6;
    CURRENT_LATCHES.REGS[DR] = Low16bits(~ CURRENT_LATCHES.REGS[SR]);

    SetCC(CURRENT_LATCHES.REGS[DR]);
    return 0;
}

int ST(int Inst){
    int SR = (Inst & 0x0E00) >> 9;
    int PCoffset9 = Inst & 0x01FF;

    int Addr = Low16bits((CURRENT_LATCHES.PC + SEXT(PCoffset9, 9)));
    MEMORY[Addr] = Low16bits(CURRENT_LATCHES.REGS[SR]);

    return 0;
}

int STI(int Inst){
    int SR = (Inst & 0x0E00) >> 9;
    int PCoffset9 = Inst & 0x01FF;

    int Addr = Low16bits((CURRENT_LATCHES.PC + SEXT(PCoffset9, 9)));
    MEMORY[MEMORY[Addr]] = Low16bits(CURRENT_LATCHES.REGS[SR]);

    return 0;
}

int STR(int Inst){
    int SR = (Inst & 0x0E00) >> 9;
    int BaseR = (Inst & 0x01C0) >> 6;
    int PCoffset6 = Inst & 0x003F;

    int Addr = Low16bits((CURRENT_LATCHES.REGS[BaseR] + SEXT(PCoffset6, 6)));
    MEMORY[Addr] = Low16bits(CURRENT_LATCHES.REGS[SR]);

    return 0;
}

int TRAP(){
    CURRENT_LATCHES.PC = 0x0000;    /* Halt */
    return 0;
}

