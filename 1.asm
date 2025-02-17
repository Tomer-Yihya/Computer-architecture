	add $r3, $zero, $imm, 127	# PC=0
loop:
	add $r4, $r4, $imm, 5	# PC=0
	lw $r2, $zero, $imm, 0		# PC=1
	add $r2, $r2, $imm, 1		# PC=2
	sw $r2, $zero, $imm, 0		# PC=3
wait:
	blt $imm, $r6, $r4, wait	# PC=4
	add $r6, $r6, $imm, 1		# PC=5
	blt $imm, $r5, $r3, loop	# PC=4
	add $r5, $r5, $imm, 1		# PC=5
	lw $r2, $zero, $imm, 256	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
	halt $zero, $zero, $zero, 0	
