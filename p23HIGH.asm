PORT2_DDR equ $0001 ;set reg location for port config
PORT2_OUT equ $0003 ;set bits for 
	
	processor HD6303

	org $E000 ;put the program at location 0000

	;set port 2 for output mode
	ldaa #$18
	staa PORT2_DDR

.1

	;; Turn on LED on P23
	ldaa #$08
	staa PORT2_OUT
	
	jmp .1