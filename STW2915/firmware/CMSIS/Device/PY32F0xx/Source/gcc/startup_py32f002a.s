/**************************************************************************//**
 * @file     startup_m031series.s
 * @version  V2.00
 * $Revision: 6 $
 * $Date: 18/04/12 4:44p $
 * @brief    CMSIS Cortex-M0 Core Device Startup File for M031
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

	.syntax	unified
	.arch	armv6-m

	.section .stack
	.align	3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0x00000100
#endif
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .heap
	.align	3
#ifdef __HEAP_SIZE
	.equ	Heap_Size, __HEAP_SIZE
#else
	.equ	Heap_Size, 0x00000000
#endif
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.if	Heap_Size
	.space	Heap_Size
	.endif
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

	.section .vectors
	.align	2
	.globl	__Vectors
__Vectors:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */

	/* External interrupts */
	.long   0                              // 0Reserved 
	.long   0                              // 1Reserved 
	.long   0                              // 2Reserved 
	.long   FLASH_IRQHandler               // 3FLASH
	.long   RCC_IRQHandler                 // 4RCC
	.long   EXTI0_1_IRQHandler             // 5EXTI Line 0 and 1
	.long   EXTI2_3_IRQHandler             // 6EXTI Line 2 and 3
	.long   EXTI4_15_IRQHandler            // 7EXTI Line 4 to 15
	.long   0                              // 8Reserved 
	.long   0                              // 9Reserved 
	.long   0                              // 10Reserved 
	.long   0                              // 11Reserved 
	.long   ADC_COMP_IRQHandler            // 12ADC&COMP1 
	.long   TIM1_BRK_UP_TRG_COM_IRQHandler // 13TIM1 Break, Update, Trigger and Commutation
	.long   TIM1_CC_IRQHandler             // 14TIM1 Capture Compare
	.long   0                              // 15Reserved 
	.long   0                              // 16Reserved 
	.long   LPTIM1_IRQHandler              // 17LPTIM1
	.long   0                              // 18Reserved 
	.long   0                              // 19Reserved 
	.long   0                              // 20Reserved 
	.long   TIM16_IRQHandler               // 21TIM16
	.long   0                              // 22Reserved 
	.long   I2C1_IRQHandler                // 23I2C1
	.long   0                              // 24Reserved 
	.long   SPI1_IRQHandler                // 25SPI1
	.long   0                              // 26Reserved
	.long   USART1_IRQHandler              // 27USART1
	.long   0                              // 28Reserved 
	.long   0                              // 29Reserved
	.long   0                              // 30Reserved
	.long   0                              // 31Reserved

	.size	__Vectors, . - __Vectors

	.text
	.thumb
	.thumb_func
	.align	2
	.globl	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r4, =__copy_table_start__
	ldr	r5, =__copy_table_end__

.L_loop0:
	cmp	r4, r5
	bge	.L_loop0_done
	ldr	r1, [r4]
	ldr	r2, [r4, #4]
	ldr	r3, [r4, #8]

.L_loop0_0:
	subs	r3, #4
	ldr	r0, [r1,r3]
	str	r0, [r2,r3]
	bge	.L_loop0_0

	adds	r4, #12
	b	.L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__etext
	ldr	r2, =__data_start__
	ldr	r3, =__data_end__

	subs	r3, r2
	ble	.L_loop1_done

.L_loop1:
	subs	r3, #4
	ldr	r0, [r1,r3]
	str	r0, [r2,r3]
	bgt	.L_loop1

.L_loop1_done:

#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
	ldr	r3, =__zero_table_start__
	ldr	r4, =__zero_table_end__

.L_loop2:
	cmp	r3, r4
	bge	.L_loop2_done
	ldr	r1, [r3]
	ldr	r2, [r3, #4]
	movs	r0, 0

.L_loop2_0:
	subs	r2, #4
	str 	r0, [r1, r2]
	bgt	.L_loop2_0

	adds	r3, #8
	b	.L_loop2
.L_loop2_done:

#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__bss_start__
	ldr	r2, =__bss_end__

    movs    r0, 0

    subs    r2, r1
    ble .L_loop3_done

.L_loop3:
    subs    r2, #4
    str r0, [r1, r2]
    bgt .L_loop3
.L_loop3_done:
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */


	// reset NVIC if in rom debug
	ldr     r0, =0x20000000
	ldr     r2, =0x0
	movs    r1, #0                 // for warning, 
	add     r1, pc,#0              // for A1609W, 
	cmp     r1, r0
	bls     .RAMCODE

	// ram code base address. 
	add     r2, r0,r2

.RAMCODE:
	// reset Vector table address.
	ldr     r0, =0xE000ED08 
	str     r2, [r0]

#ifndef __NO_SYSTEM_INIT
	bl	SystemInit
#endif
	
#ifndef __START
#define __START _start
#endif
	bl	__START

//	 bl  main
//     bx  lr

	.pool
	.size	Reset_Handler, . - Reset_Handler

	.align	1
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size	Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm




    def_irq_handler	NMI_Handler
    def_irq_handler	HardFault_Handler
    def_irq_handler	SVC_Handler
    def_irq_handler	PendSV_Handler
    def_irq_handler	SysTick_Handler
	def_irq_handler FLASH_IRQHandler              
	def_irq_handler RCC_IRQHandler                
	def_irq_handler EXTI0_1_IRQHandler             
	def_irq_handler EXTI2_3_IRQHandler             
	def_irq_handler EXTI4_15_IRQHandler   
	def_irq_handler ADC_COMP_IRQHandler          
	def_irq_handler TIM1_BRK_UP_TRG_COM_IRQHandler
	def_irq_handler TIM1_CC_IRQHandler
	def_irq_handler LPTIM1_IRQHandler
	def_irq_handler TIM16_IRQHandler
	def_irq_handler I2C1_IRQHandler
	def_irq_handler SPI1_IRQHandler
	def_irq_handler USART1_IRQHandler


    .end
