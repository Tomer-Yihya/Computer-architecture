	add $r3, $zero, $imm, 128	# PC=0
loop:
	lw $r2, $zero, $imm, 0		# PC=1
	add $r2, $r2, $imm, 1		# PC=2
	sw $r2, $zero, $imm, 0		# PC=3
	blt $imm, $r5, $r3, loop	# PC=4
	add $r5, $r5, $imm, 1		# PC=5
	lw $r2, $zero, $imm, 256	# PC=6
	halt $zero, $zero, $zero, 0	# PC=7
	halt $zero, $zero, $zero, 0	# PC=8
	halt $zero, $zero, $zero, 0	# PC=9
	halt $zero, $zero, $zero, 0	# PC=10
	halt $zero, $zero, $zero, 0	# PC=11
