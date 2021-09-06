.global main
main:
	li s0, 1
	li s1, 101
	li t0, 0
	bge s0, s1, end
loop:
	add t0, t0, s0
	addi s0, s0, 1
	blt s0, s1, loop
end:
	j end
