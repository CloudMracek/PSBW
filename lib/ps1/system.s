# ps1-bare-metal - (C) 2023 spicyjpeg
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

.set noreorder
.set noat

## Exception handler

.set BADV,  $8
.set SR,    $12
.set CAUSE, $13
.set EPC,   $14

.section .text._exceptionVector, "ax", @progbits
.global _exceptionVector
.type _exceptionVector, @function

_exceptionVector:
	# This tiny stub is going to be relocated to the address the CPU jumps to
	# when an exception occurs (0x80000080) at runtime, overriding the default
	# one installed by the BIOS. We're going to fetch a pointer to the current
	# thread, grab the EPC (i.e. the address of the instruction that was being
	# executed before the exception occurred) and jump to the exception handler.
	# NOTE: we can't use any registers other than $k0 and $k1 here, as doing so
	# would destroy their contents and corrupt the current thread's state.
	lui   $k0, %hi(currentThread)
	lw    $k0, %lo(currentThread)($k0)

	j     _exceptionHandler
	mfc0  $k1, EPC

.section .text._exceptionHandler, "ax", @progbits
.global _exceptionHandler
.type _exceptionHandler, @function

_exceptionHandler:
	# Save the full state of the thread in order to make sure the interrupt
	# handler callback (invoked later on) can use any register. The state of
	# $hi/$lo is saved after all other registers in order to let the multiplier
	# finish any ongoing calculation.
	sw    $at, 0x04($k0)
	sw    $v0, 0x08($k0)
	sw    $v1, 0x0c($k0)
	sw    $a0, 0x10($k0)
	sw    $a1, 0x14($k0)
	sw    $a2, 0x18($k0)
	sw    $a3, 0x1c($k0)
	sw    $t0, 0x20($k0)
	sw    $t1, 0x24($k0)
	sw    $t2, 0x28($k0)
	sw    $t3, 0x2c($k0)
	sw    $t4, 0x30($k0)
	sw    $t5, 0x34($k0)
	sw    $t6, 0x38($k0)
	sw    $t7, 0x3c($k0)
	sw    $s0, 0x40($k0)
	sw    $s1, 0x44($k0)
	sw    $s2, 0x48($k0)
	sw    $s3, 0x4c($k0)
	sw    $s4, 0x50($k0)
	sw    $s5, 0x54($k0)
	sw    $s6, 0x58($k0)
	sw    $s7, 0x5c($k0)
	sw    $t8, 0x60($k0)
	sw    $t9, 0x64($k0)
	sw    $gp, 0x68($k0)
	sw    $sp, 0x6c($k0)
	sw    $fp, 0x70($k0)
	sw    $ra, 0x74($k0)

	mfhi  $v0
	mflo  $v1
	sw    $v0, 0x78($k0)
	sw    $v1, 0x7c($k0)

	# Check bits 2-6 of the CAUSE register to determine what triggered the
	# exception. If it was caused by a syscall, increment EPC to make sure
	# returning to the thread won't trigger another syscall.
	mfc0  $v0, CAUSE
	lui   $v1, %hi(interruptHandler)

	andi  $v0, 0x1f << 2 # if (((CAUSE >> 2) & 0x1f) == 0) goto checkForGTEInst
	beqz  $v0, .LcheckForGTEInst
	li    $at, 8 << 2 # if (((CAUSE >> 2) & 0x1f) == 8) goto applyIncrement
	beq   $v0, $at, .LapplyIncrement
	lw    $v1, %lo(interruptHandler)($v1)

.LotherException:
	# If the exception was not triggered by a syscall nor by an interrupt call
	# _unhandledException(), which will then display information about the
	# exception and lock up.
	sw    $k1, 0x00($k0)

	mfc0  $a1, BADV # _unhandledException((CAUSE >> 2) & 0x1f, BADV)
	srl   $a0, $v0, 2
	jal   _unhandledException
	addiu $sp, -8

	b     .Lreturn
	addiu $sp, 8

.LcheckForGTEInst:
	# If the exception was caused by an interrupt, check if the interrupted
	# instruction was a GTE opcode and increment EPC to avoid executing it again
	# if that is the case. This is a workaround for a hardware bug.
	lw    $v0, 0($k1) # if ((*EPC >> 25) == 0x25) EPC++
	li    $at, 0x25
	srl   $v0, 25
	bne   $v0, $at, .LskipIncrement
	lw    $v1, %lo(interruptHandler)($v1)

.LapplyIncrement:
	addiu $k1, 4

.LskipIncrement:
	# Save the modified EPC and dispatch any pending interrupts. The handler
	# will temporarily use the current thread's stack.
	sw    $k1, 0x00($k0)

	lui   $a0, %hi(interruptHandlerArg)
	lw    $a0, %lo(interruptHandlerArg)($a0)
	jalr  $v1 # interruptHandler(interruptHandlerArg)
	addiu $sp, -8

	addiu $sp, 8

.Lreturn:
	# Grab a pointer to the next thread to be executed, restore its state and
	# return.
	lui   $k0, %hi(nextThread)
	lw    $k0, %lo(nextThread)($k0)
	lui   $at, %hi(currentThread)
	sw    $k0, %lo(currentThread)($at)

	lw    $v0, 0x78($k0)
	lw    $v1, 0x7c($k0)
	mthi  $v0
	mtlo  $v1

	lw    $k1, 0x00($k0)
	lw    $at, 0x04($k0)
	lw    $v0, 0x08($k0)
	lw    $v1, 0x0c($k0)
	lw    $a0, 0x10($k0)
	lw    $a1, 0x14($k0)
	lw    $a2, 0x18($k0)
	lw    $a3, 0x1c($k0)
	lw    $t0, 0x20($k0)
	lw    $t1, 0x24($k0)
	lw    $t2, 0x28($k0)
	lw    $t3, 0x2c($k0)
	lw    $t4, 0x30($k0)
	lw    $t5, 0x34($k0)
	lw    $t6, 0x38($k0)
	lw    $t7, 0x3c($k0)
	lw    $s0, 0x40($k0)
	lw    $s1, 0x44($k0)
	lw    $s2, 0x48($k0)
	lw    $s3, 0x4c($k0)
	lw    $s4, 0x50($k0)
	lw    $s5, 0x54($k0)
	lw    $s6, 0x58($k0)
	lw    $s7, 0x5c($k0)
	lw    $t8, 0x60($k0)
	lw    $t9, 0x64($k0)
	lw    $gp, 0x68($k0)
	lw    $sp, 0x6c($k0)
	lw    $fp, 0x70($k0)
	lw    $ra, 0x74($k0)

	jr    $k1
	rfe

## Delay functions

.set IO_BASE, 0xbf801000

.set TIMER2_VALUE,  IO_BASE | 0x120
.set TIMER2_CTRL,   IO_BASE | 0x124
.set TIMER2_RELOAD, IO_BASE | 0x128

.section .text.delayMicroseconds, "ax", @progbits
.global delayMicroseconds
.type delayMicroseconds, @function

delayMicroseconds:
	# Calculate the approximate number of CPU cycles that need to be burned,
	# assuming a 33.8688 MHz clock (1 us = 33.8688 = ~33.875 cycles).
	sll   $a1, $a0, 8 # cycles = ((us * 271) + 4) / 8
	sll   $a2, $a0, 4
	addu  $a1, $a2
	subu  $a1, $a0
	addiu $a1, 4
	sra   $a0, $a1, 3

	# Compensate for the overhead of calculating the cycle count, entering the
	# loop and returning.
	addiu $a0, -(6 + 1 + 2 + 4 + 2)

	# Reset timer 2 to its default setting of counting system clock edges.
	lui   $v1, %hi(IO_BASE)
	sh    $0,  %lo(TIMER2_CTRL)($v1) # TIMER2_CTRL = 0

	# Wait for up to 0xff00 cycles at a time (resetting the timer and waiting
	# for it to count up each time), as the counter is only 16 bits wide. We
	# have to wait 0xff00 cycles rather than 0x10000 since the counter wraps
	# around rather than saturating on overflow.
	li    $a1, 0xff00
	slt   $v0, $a1, $a0
	beqz  $v0, .LshortDelay
	li    $a2, 0xff00 + 3

.LlongDelay: # for (; cycles > 0xff00; cycles -= (0xff00 + 3))
	sh    $0,  %lo(TIMER2_VALUE)($v1) # TIMER2_VALUE = 0
	li    $v0, 0

.LlongDelayLoop: # while (TIMER2_VALUE < 0xff00);
	nop
	slt   $v0, $v0, $a1
	bnez  $v0, .LlongDelayLoop
	lhu   $v0, %lo(TIMER2_VALUE)($v1)

	slt   $v0, $a1, $a0
	bnez  $v0, .LlongDelay
	subu  $a0, $a2

.LshortDelay:
	# Run the last busy loop once less than 0xff00 cycles are remaining.
	sh    $0,  %lo(TIMER2_VALUE)($v1) # TIMER2_VALUE = 0
	li    $v0, 0

.LshortDelayLoop: # while (TIMER2_VALUE < cycles);
	nop
	slt   $v0, $v0, $a0
	bnez  $v0, .LshortDelayLoop
	lhu   $v0, %lo(TIMER2_VALUE)($v1)

	jr    $ra
	nop

.section .text.delayMicrosecondsBusy, "ax", @progbits
.global delayMicrosecondsBusy
.type delayMicrosecondsBusy, @function

delayMicrosecondsBusy:
	# Calculate the approximate number of CPU cycles that need to be burned,
	# assuming a 33.8688 MHz clock (1 us = 33.8688 = ~33.875 cycles).
	sll   $a1, $a0, 8 # cycles = ((us * 271) + 4) / 8
	sll   $a2, $a0, 4
	addu  $a1, $a2
	subu  $a1, $a0
	addiu $a1, 4
	sra   $a0, $a1, 3

	# Compensate for the overhead of calculating the cycle count and returning.
	addiu $a0, -(6 + 1 + 2)

.Lloop: # while (cycles > 0) cycles -= 2
	bgtz  $a0, .Lloop
	addiu $a0, -2

	jr    $ra
	nop
