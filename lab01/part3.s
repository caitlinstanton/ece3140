    AREA Myprog, CODE, READONLY
    ENTRY
    EXPORT __main

;don't change these addresses!
PCR22     EQU 0x4004A058 ;PORTB_PCR22  address
SCGC5     EQU 0x40048038 ;SIM_SCGC5    address
PDDR    EQU 0x400FF054 ;GPIOB_PDDR   address
PCOR    EQU 0x400FF048 ;GPIOB_PCOR   address
PSOR      EQU 0x400FF044 ;GPIOB_PSOR   address

ten     EQU 0x00000400 ; 1 << 10
eight     EQU 0x00000100 ; 1 << 8
twentytwo EQU 0x00400000 ; 1 << 22

__main
  ; Your code goes here!
    BL    LEDSETUP

    MOV   R0, #-20
	BL    fib
    BL    morseDigit
	; ouput 0

    BL    space
    BL    space
    BL    space

    MOV   R0, #-1
	BL    fib
    BL    morseDigit
    ; ouput 0

    BL    space
    BL    space
    BL    space

    MOV   R0, #0
	BL    fib
    BL    morseDigit
    ; ouput 0

    BL    space
    BL    space
    BL    space

    MOV   R0, #1
	BL    fib
    BL    morseDigit
    ; ouput 1

    BL    space
    BL    space
    BL    space

    MOV   R0, #2
	BL    fib
    BL    morseDigit
    ; ouput 1

    BL    space
    BL    space
    BL    space

    MOV   R0, #3
	BL    fib
    BL    morseDigit
    ; ouput 2

    BL    space
    BL    space
    BL    space

    MOV   R0, #4
	BL    fib
    BL    morseDigit
    ; ouput 3

    BL    space
    BL    space
    BL    space

    MOV   R0, #5
	BL    fib
    BL    morseDigit
    ; ouput 5

    BL    space
    BL    space
    BL    space

    MOV   R0, #6
	BL    fib
    BL    morseDigit
    ; ouput 8

fib
        PUSH  {R4-R5, LR}    ; preserve R4 and R5
        CMP   R0, #1
        BGT   gt1
        MOVLT R0, #0         ; n = 0
        MOVEQ R0, #1         ; n = 1
        B     end_fib
gt1                      ; n >= 1
        MOV   R4, R0       ; save n to R4
        SUB   R0, #1
        BL    fib      ; fib (n-1)
        MOV   R5, R0       ; save fib (n-1) to R5
        SUB   R0, R4, #2
        BL    fib      ; fib (n-2)
        ADD   R0, R5       ; fib (n) = fib (n-1) + fib (n-2)
end_fib
        POP   {R4-R5, LR}
        BX    LR

; Call this function first to set up the LED
LEDSETUP
        PUSH  {R4, R5} ; To preserve R4 and R5
        LDR   R4, =ten ; Load the value 1 << 10
        LDR   R5, =SCGC5
        STR   R4, [R5]

        LDR   R4, =eight
        LDR   R5, =PCR22
        STR   R4, [R5]

        LDR   R4, =twentytwo
        LDR   R5, =PDDR
        STR   R4, [R5]
        POP   {R4, R5}
        BX    LR

; The functions below are for you to use freely
LEDON
        PUSH  {R4, R5}
        LDR   R4, =twentytwo
        LDR   R5, =PCOR
        STR   R4, [R5]
        POP   {R4, R5}
        BX    LR
LEDOFF
        PUSH  {R4, R5}
        LDR   R4, =twentytwo
        LDR   R5, =PSOR
        STR   R4, [R5]
        POP   {R4, R5}
        BX    LR

    ; function to display multi digits in morse code
morseMulti
        PUSH  {LR}
        MOV   R1, #-1       ; R1 is -1
        PUSH  {R1}            ; Add -1 to stack as flag
        LDR   R2, =0x1999999A     ; R2 is 0x1999999A [2^(32) / 10]
        MOV   R3, #10             ; R3 is 10
    ; extract digits and add to stack.
    ; starts with adding lower place digits and then higher ones
loop_digit
        SDIV  R5, R0, R3        ; R5 is R0 / 10
        MUL   R4, R5, R3        ; R4 is now 10 * (R0 / 10)
        SUB   R4, R0, R4          ; R4 is now one's digit of R0
        MOV   R1, R4
        PUSH  {R1}
        MOVS  R0, R5
        BNE   loop_digit
    ; display digits using the stack to reverse order
loop_display
        POP {R1}
        MOV R0, R1
        CMP R1, #-1
        BEQ done_multi        ; stop if flag reached, no more digits to display
        BL morseDigit
        BL space
        BL space
        B  loop_display
done_multi
        POP {LR}
        BX LR

   ; function to display a single digit in morse code
morseDigit
        PUSH  {LR}
        CMP   R0, #5
        BGT   n_gt5
        ; here n <= 5
        ; number of dots in n :  R0 = n
        ; number of dashs in n : R1 = 5 - n
        ADD   R1, R0, #-5
        NEG   R1, R1
loop_dot1
        CMP   R0, #0
        BLE   loop_dash1
        BL    dot
        SUBS  R0, #1
        B     loop_dot1
loop_dash1
        CMP   R1, #0
        BLE   done
        BL    dash
        SUBS  R1, #1
        B     loop_dash1
n_gt5
        ; here n > 5
        ; number of dashs in n : n - 5
        SUB   R0, R0, #5
        ; number of dots in n : 5 - (n - 5)
        ADD   R1, R0, #-5
        NEG   R1, R1
loop_dash2
        CMP   R0, #0
        BLE   loop_dot2
        BL    dash
        SUBS  R0, #1
        B     loop_dash2
loop_dot2
        CMP   R1, #0
        BLE   done
        BL    dot
        SUBS  R1, #1
        B     loop_dot2
done
        POP   {LR}
        BX    LR

    ; function to display a single dash with LED
dash
        PUSH  {LR}
        BL    LEDON
        BL    space
        BL    space
        BL    space
        BL    LEDOFF
        BL    space
        POP   {LR}
        BX    LR

    ; function to display a single dot with LED
dot
        PUSH  {LR}
        BL    LEDON
        BL    space
        BL    LEDOFF
        BL    space
        POP   {LR}
        BX    LR

    ; function to create a delay between LED on / off
space
        PUSH  {LR}
        LDR   R2, =0x2DC6C0       ; set R2 to an arbitary large value
loop
        SUBS  R2, #1
        BNE   loop
        POP   {LR}
        BX    LR

forever
        B   forever           ; wait here forever
        END
