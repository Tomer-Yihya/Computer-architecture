	add $r2, $zero, $imm, 255
	add $r3, $zero, $imm, 0
	add $r4, $r3, $imm, 4095
	add $r4, $r4, $imm, 1
	add $r5, $r4, $imm, 4095
	add $r5, $r5, $imm, 1
loop:
	lw $r6, $r3, $imm, 0
	lw $r7, $r3, $imm, 1		
	lw $r8, $r3, $imm, 2		
	lw $r9, $r3, $imm, 3		
	lw $r10, $r4, $imm, 0		
	lw $r11, $r4, $imm, 1		
	lw $r12, $r4, $imm, 2		
	lw $r13, $r4, $imm, 3		
	add $r6, $r6, $r10
	add $r7, $r7, $r11
	add $r8, $r8, $r12
	add $r9, $r9, $r13
	sw $r6, $r5, $imm, 0
	sw $r7, $r5, $imm, 1
	sw $r8, $r5, $imm, 2
	sw $r9, $r5, $imm, 3
	add $r3, $r3, $imm, 16	
	add $r4, $r4, $imm, 16	
	add $r5, $r5, $imm, 16	
	blt $imm, $r14, $r2, loop	
	add $r14, $r14, $imm, 1	
	add $r2, $zero, $imm, 15
	add $r14, $zero, $imm, 0	
	add $r3, $zero, $imm, 0
loop2:
	lw $r4, $r3, $imm, 0
	add $r3, $r3, $imm, 16
	blt $imm, $r14, $r2, loop2	
	add $r14, $r14, $imm, 1	
	halt $zero, $zero, $zero, 0
	halt $zero, $zero, $zero, 0
	halt $zero, $zero, $zero, 0
	halt $zero, $zero, $zero, 0
	halt $zero, $zero, $zero, 0
