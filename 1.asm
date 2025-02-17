	add $r3, $zero, $imm, 127
loop:
	add $r4, $r4, $imm, 5
	lw $r2, $zero, $imm, 0
	add $r2, $r2, $imm, 1
	sw $r2, $zero, $imm, 0
wait:
	blt $imm, $r6, $r4, wait
	add $r6, $r6, $imm, 1
	blt $imm, $r5, $r3, loop
	add $r5, $r5, $imm, 1
	lw $r2, $zero, $imm, 256	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
