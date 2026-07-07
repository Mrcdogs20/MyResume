#include "spimcore.h"

// MIPS Simulator Project
// Group 21 
// November 21, 2025

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
	//000: Perform Addition,  A + B
    if (ALUControl == 0) {
        *ALUresult = A + B;
    }

    //001: Perform Subtraction, A - B
    else if (ALUControl == 1) {
        *ALUresult = A - B;
    }

    //010: Set-on-Less-Than (signed). Result is 1 if A < B. Otherwise 0
    else if (ALUControl == 2) {
        if ((int)A < (int)B) {
            *ALUresult = 1;
        } else {
            *ALUresult = 0;
        }
    }

    //011: Set-on-Less-Than (unsigned). Result is 1 if A < B. Otherwise 0.
    else if (ALUControl == 3) {
        if (A < B) {
            *ALUresult = 1;
        } else {
            *ALUresult = 0;
        }
    }

    //100: AND: A & B
    else if (ALUControl == 4) {
        *ALUresult = A & B;
    }

    //101: OR: A | B
    else if (ALUControl == 5) {
        *ALUresult = A | B;
    }

    //110: Left Shift B by 16 bits
    else if (ALUControl == 6) {
        *ALUresult = B << 16;
    }

    //111: NOT: ~A
    else if (ALUControl == 7) {
        *ALUresult = ~A;
    }

    //Set Zero a flag if ALUresult is 0.
    if (*ALUresult == 0) {
        *Zero = 1;
    } else {
        *Zero = 0;
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
    // Ensure that the Program Counter (PC) is word-aligned. It must be divisible by 4 (since 1 word = 4 bytes).
    if (PC % 4 != 0)
        return 1;

    // Convert byte address to word index. Memory is 64KB, which is 65536 bytes / 4 bytes per word = 16384 words
    unsigned index = PC >> 2;

    if (index > 16383)
        return 1;
	
	// Fetch the instruction from memory
    *instruction = Mem[index];

    // If value for instruction is 0, we halt (illegal instruction). 
    if (*instruction == 0)
        return 1;

    return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	// Extract the opcode field (bits 31–26).
    // Shift the instruction right by 26 bits so the opcode moves into the lowest 6 bits,
    // then mask with 0x3F (binary 111111) to keep only those 6 bits.
    *op = (instruction >> 26) & 0x3F;


	// Extract the rs (1st register source) field (bits 25–21).
    // Shift right by 21 bits so rs is in the lowest 5 bits,
    // then mask with 0x1F (binary 11111) to keep only those 5 bits.
    *r1 = (instruction >> 21) & 0x1F;

    
    // Extract the rt (2nd register source) field (bits 20–16).
    // Shift right by 16 bits so rt is in the lowest 5 bits,
    // then mask with 0x1F to isolate those 5 bits.
    *r2 = (instruction >> 16) & 0x1F;


    // Extract the rd (register destination) field (bits 15–11).
    // Shift right by 11 bits so rd is in the lowest 5 bits,
    // then mask with 0x1F to keep only those 5 bits.
    *r3 = (instruction >> 11) & 0x1F;

    // Extract the funct field (bits 5–0).
    // No shifting is needed. Mask with 0x3F (6 bits) to isolate the function code.
    *funct = instruction & 0x3F;

    // Extract the immediate offset field (bits 15–0).
    // Mask with 0xFFFF (16 bits) to keep only the lower half of the instruction.
    *offset = instruction & 0xFFFF;

	// Extract the jump section field (bits 25–0).
    // Mask with 0x3FFFFFF (26 bits) to keep only the lowest 26 bits.
    *jsec = instruction & 0x03FFFFFF;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // R-type instructions: add, sub, and, or, slt, sltu 
    // opcode = 0x00
    if (op == 0x00) {
        controls->RegDst   = 1;  // write to register destination
        controls->ALUSrc   = 0;  // second ALU operand from the register
        controls->MemtoReg = 0;  // write ALU result
        controls->RegWrite = 1;  // allow register write
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 7;  // use funct to determine ALU operation
        return 0;
    }

    // addi: opcode = 0x08
    else if (op == 0x08) {
        controls->RegDst   = 0;  // write to second register source (rt)
        controls->ALUSrc   = 1;  // immediate operand
        controls->MemtoReg = 0;  // result from the ALU
        controls->RegWrite = 1;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 0;  // add
        return 0;
    }

    // and & or are R-type, no separate opcode cases needed.

    // lw: opcode = 0x23
    else if (op == 0x23) {
        controls->RegDst   = 0;  // rt
        controls->ALUSrc   = 1;  // base address + the offset
        controls->MemtoReg = 1;  // load memory data
        controls->RegWrite = 1;
        controls->MemRead  = 1;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 0;  // add
        return 0;
    }

    // sw: opcode = 0x2B
    else if (op == 0x2B) {
        controls->RegDst   = 2;  // we don't care
        controls->ALUSrc   = 1;  // base address + offset
        controls->MemtoReg = 2;  // we don't care
        controls->RegWrite = 0;
        controls->MemRead  = 0;
        controls->MemWrite = 1;	 // store the word
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 0;  // add
        return 0;
    }

    // lui: opcode = 0x0F
    else if (op == 0x0F) {
        controls->RegDst   = 0;  // rt
        controls->ALUSrc   = 1;  // immediate left shift by 16
        controls->MemtoReg = 0;  // ALU result
        controls->RegWrite = 1;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 6;  // shift left by 16
        return 0;
    }

    // beq: opcode = 0x04
    else if (op == 0x04) {
        controls->RegDst   = 2;  // we don't care
        controls->ALUSrc   = 0;  // register operands
        controls->MemtoReg = 2;  // we don't care
        controls->RegWrite = 0;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 1;  // allow branch
        controls->Jump     = 0;
        controls->ALUOp    = 1;  // subtract
        return 0;
    }

    // slti (signed): opcode = 0x0A
    else if (op == 0x0A) {
        controls->RegDst   = 0;  // rt
        controls->ALUSrc   = 1;  // immediate
        controls->MemtoReg = 0;  // ALU result
        controls->RegWrite = 1;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 2;  // signed less-than
        return 0;
    }

    // sltiu (unsigned): opcode = 0x0B
    else if (op == 0x0B) {
        controls->RegDst   = 0;  // rt
        controls->ALUSrc   = 1;  // immediate
        controls->MemtoReg = 0;  // ALU result
        controls->RegWrite = 1;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 0;
        controls->ALUOp    = 3;  // unsigned less-than
        return 0;
    }

    // jump: opcode = 0x02
    else if (op == 0x02) {
        controls->RegDst   = 2;  // we don't care
        controls->ALUSrc   = 2;  // we don't care
        controls->MemtoReg = 2;  // we don't care
        controls->RegWrite = 0;
        controls->MemRead  = 0;
        controls->MemWrite = 0;
        controls->Branch   = 0;
        controls->Jump     = 1;  // allow jump
        controls->ALUOp    = 0;  // we don't care
        return 0;
    }

    // illegal instruction
    return 1;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	 // Read register r1 and store into data1
    *data1 = Reg[r1];

    // Read register r2 and store into data2
    *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	// Check if the most significant bit (bit 15) of the 16-bit offset is 1.
	if (offset & 0x8000) {
		
		// If the sign bit is 1, the number is negative.
        // We extend the sign by filling the upper 16 bits with 1s.
		*extended_value = offset | 0xFFFF0000;
	} else {
		
		// If the sign bit is 0, the number is positive.
        // Set the upper 16 bits to 0 to ensure the value stays positive.
		*extended_value = offset & 0x0000FFFF;
	}

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	 unsigned B;
	
	// Select the second operand based on ALUSrc
    if (ALUSrc == 0){
        B = data2;			// register operand
    }
    else if (ALUSrc == 1){
        B = extended_value;		// immediate operand
    }
    else {
        B = data2;			
    }

    char final_ALUControl = 0;

    if (ALUOp == 7){

        //Running these functions all with the same format in the order I did for ALU
        // R-type instructions: ALUOp = 7, funct field decides operation
        
        //000: Z = A + B
        if (funct == 0x20){ 		 // add
            final_ALUControl = 0;	
        }
        //001: Z = A - B    
        else if (funct == 0x22){	// subtract 
            final_ALUControl = 1; 	
        }
        //010: signed A < B
        else if (funct == 0x2A){	// slt (signed)
            final_ALUControl = 2;	
        }
        //011: unsigned A < B
        else if (funct == 0x2B){	// sltu (unsigned)
            final_ALUControl = 3; 	
        }
        //100: A AND B
        else if (funct == 0x24){	// and
            final_ALUControl = 4;
        }
        //101: A OR B
        else if (funct == 0x25){	// or
            final_ALUControl = 5;
        }

        else{
            return 1; // unsupported funct
        }
    }
	
	// Immediate / I-type instructions: ALUOp directly maps to ALUControl
    else if (ALUOp == 0){
        final_ALUControl = 0;		// add (addi, lw, sw)
    }
    else if (ALUOp == 1){
        final_ALUControl = 1;		// sub (beq)
    }
    else if (ALUOp == 2){
        final_ALUControl = 2;		// slti (signed)
    }
    else if (ALUOp == 3){
        final_ALUControl = 3;		// sltiu (unsigned)
    }
    else if (ALUOp == 6){
        final_ALUControl = 6;		// lui (shift left 16)
    }
    else{
        final_ALUControl = ALUOp;
    }
	
	// perform the ALU operation
    ALU(data1, B, final_ALUControl, ALUresult, Zero);

    return 0; // success 
	
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{	
	// If neither read nor write is requested, nothing to do
    if (!MemRead && !MemWrite)
        return 0; 
        
    // Convert byte address to word index (memory is addressed based on words)
    unsigned index = ALUresult >> 2;

	// Check memory bounds (64 KB = 16384 words)
    if (index > 16383)
        return 1;

     // Address must be word-aligned
    if (ALUresult % 4 != 0)
        return 1;

    // LOAD: read Mem[index], and put it into memdata
    if (MemRead)
        *memdata = Mem[index];

    // STORE: write data2 into memory
    if (MemWrite)
        Mem[index] = data2;

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	// If RegWrite = 0, we do not need to do anything.
    if (!RegWrite) {
        return;
    }

    // Determine which register to write to:
    unsigned dest;

    if (RegDst == 0) {
        dest = r2;       // This is an I-type: destination is rt
    }
    else if (RegDst == 1) {
        dest = r3;       // This is an R-type: destination is rd
    }
    else {
        return;          // "Don't care", do nothing
    }
    
    // Prevent writes to register $0 (hardwired to 0 in MIPS)
    if (dest == 0) {
        return;          
	}
	
    // Determine what data to write:
    unsigned value;

    if (MemtoReg == 0) {
        value = ALUresult;   // Write result from ALU
    }
    else if (MemtoReg == 1) {
        value = memdata;     // Write data read from memory
    }
    else {
        return;              // "Don't care"
    }

    // Write to the register file
    Reg[dest] = value;

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
	// Calculate the next PC address for normal sequential execution (PC + 4)
    unsigned next_pc = *PC + 4;
    
    // Handle Branch instructions (beq)
    // Branch if Branch signal is enabled AND Zero flag is set (for beq)
    if (Branch == 1 && Zero == 1) {
        // Sign-extend the offset and shift left by 2 to get byte offset
        // Note: extended_value is already sign-extended, so we just need to shift
        unsigned branch_offset = extended_value << 2;
        
        // Calculate branch target: PC + 4 + (offset << 2)
        unsigned branch_target = *PC + 4 + branch_offset;
        
        // Use branch target as next PC
        next_pc = branch_target;
    }
    
    // Handle Jump instructions (j)
    if (Jump == 1) {
        // For jump instructions:
        // - Take the 26-bit jsec field
        // - Shift left by 2 to get byte address (since instructions are word-aligned)
        // - Combine with upper 4 bits of PC+4 to form full 32-bit address
        unsigned jump_target = ((*PC + 4) & 0xF0000000) | (jsec << 2);
        
        // Use jump target as next PC
        next_pc = jump_target;
    }
    
    // Update the PC with the calculated next address
    *PC = next_pc;
}

