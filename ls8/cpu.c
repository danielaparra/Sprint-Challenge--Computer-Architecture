#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_LEN 6

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, int argc, char** argv)
{
  FILE *fp;
  char line[1024];

  if (argc != 2) {
    printf("usage: fileio filename\n");
    return;
  }

  fp = fopen(argv[1], "r");

  if (fp == NULL) {
    printf("Error opening file %s\n", argv[1]);
  }
  
  int address = 0;
  while (fgets(line, 1024, fp) != NULL) {
    
    // Convert string to a number
    char *endchar;
    unsigned char byte = strtol(line, &endchar, 2);
    ;

    // Ignore lines from which no numbers were read
    if (endchar == line) {
      continue;
    }
    
    cpu->ram[address] = byte;
    address++;
  }

  fclose(fp);  
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      cpu->registers[regA] *= cpu->registers[regB];
      cpu->PC += 3;
      break;

    case ALU_ADD:
      cpu->registers[regA] += cpu->registers[regB];
      cpu->PC += 3;
      break;

    case ALU_CMP:
      if (cpu->registers[regA] == cpu->registers[regB]) {
		    cpu->flags[F7_EQUAL] = 1;
	    } else {
		    cpu->flags[F7_EQUAL] = 0;
	    }
	    cpu->PC += 3;
      break;
    case ALU_AND:
      //unsigned char temp = cpu->registers[operandA] & cpu->registers[operandB];
	    cpu->registers[regA] = cpu->registers[regA] & cpu->registers[regB];
	    cpu->PC += 3;
      break;
    case ALU_OR:
	    cpu->registers[regA] = cpu->registers[regA] | cpu->registers[regB];
	    cpu->PC += 3;
	    break;
    case ALU_XOR:
      cpu->registers[regA] = cpu->registers[regA] ^ cpu->registers[regB];
	    cpu->PC += 3;
	    break;
    case ALU_NOT:
      cpu->registers[regA] = ~(cpu->registers[regA]);
	    cpu->PC += 2;
	    break;
    case ALU_SHL:
      cpu->registers[regA] =  cpu->registers[regA] << cpu->registers[regB];
	    cpu->PC += 3;
	    break;
    case ALU_SHR:
      cpu->registers[regA] = cpu->registers[regA] >> cpu->registers[regB];
	    cpu->PC += 3;
	    break;
    case ALU_MOD:
      if (cpu->registers[regB] != 0) {
	      cpu->registers[regA] %= cpu->registers[regB];
	    } else {
		    printf("Error: Cannot modulo 0");
		    // call HLT
        cpu->ram[cpu->PC + 3] = HLT;
	    }
      cpu->PC += 3;
	    break;
  }
}

/**
 * Helper Functions
 */

/**
 * Read and return ram at address
 */
unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address) {
  return cpu->ram[address];
}

/**
 * Write value to ram at address
 */
void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value) {
  cpu->ram[address] = value;
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  while (running) {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    unsigned char IR = cpu_ram_read(cpu, cpu->PC);
    
    // 2. Figure out how many operands this next instruction requires
    int num_of_operands = IR>>6;

    // 3. Get the appropriate value(s) of the operands following this instruction
    unsigned char operandA, operandB;

    switch (num_of_operands) {
      case 1:
        operandA = cpu_ram_read(cpu, cpu->PC + 1);
      case 2:
        operandA = cpu_ram_read(cpu, cpu->PC + 1);
        operandB = cpu_ram_read(cpu, cpu->PC + 2);
      default:
        break;
    }

    //printf("TRACE: %x, %d\n", IR, cpu->PC);

    // 4. switch() over it to decide on a course of action.
    switch (IR) {
      // 5. Do whatever the instruction should do according to the spec.
      // 6. Move the PC to the next instruction.
      case LDI:
        cpu->registers[operandA] = operandB;
        cpu->PC += 3;
        break;
      case PRN:
        printf("%d\n", cpu->registers[operandA]);
        cpu->PC += 2;
        break;
      case MUL:
        alu(cpu, ALU_MUL, operandA, operandB);
        break;
      case ADD: 
        alu(cpu, ALU_ADD, operandA, operandB);
        break;
      case CMP:
        alu(cpu, ALU_CMP, operandA, operandB);
        break;
      case AND:
        alu(cpu, ALU_AND, operandA, operandB);
        break;
      case OR:
        alu(cpu, ALU_OR, operandA, operandB);
        break;
      case XOR:
        alu(cpu, ALU_XOR, operandA, operandB);
        break;
      case NOT:
        alu(cpu, ALU_NOT, operandA, 0);
        break;
      case SHL:
        alu(cpu, ALU_SHL, operandA, operandB);
        break;
      case SHR:
        alu(cpu, ALU_SHR, operandA, operandB);
        break;
      case MOD:
        alu(cpu, ALU_MOD, operandA, operandB);
        break;
      case POP:
        cpu->registers[operandA] = cpu_ram_read(cpu, cpu->registers[R7]);
        cpu->registers[R7]++;
        cpu->PC += 2;
        break;
      case PUSH:
        cpu->registers[R7]--;
        cpu_ram_write(cpu, cpu->registers[R7], cpu->registers[operandA]);
        cpu->PC += 2;
        break;
      case CALL:
        cpu->registers[R7]--;
        cpu_ram_write(cpu, cpu->registers[R7], cpu->PC + 2);
        cpu->PC = cpu->registers[operandA];
        break;
      case RET:
        cpu->PC = cpu_ram_read(cpu, cpu->registers[R7]);
        cpu->registers[R7]++;
        break;
      case JMP:
        cpu->PC = cpu->registers[operandA];
	      break;
      case JEQ:
	      if (cpu->flags[F7_EQUAL] == 1) {
		      cpu->PC = cpu->registers[operandA];
	      } else {
		      cpu->PC += 2;
	      }
	      break;
      case JNE:
	      if (cpu->flags[F7_EQUAL] == 0) {
		      cpu->PC = cpu->registers[operandA];
	      } else {
		      cpu->PC += 2;
	      }
	      break;
      case HLT:
        running = 0;
        cpu->PC++;
        break;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  cpu->PC = 0;
  memset(cpu->ram, 0, 256);
  memset(cpu->registers, 0, 8);
  memset(cpu->flags, 0, 8);
  cpu->registers[R7] = SP; 
}