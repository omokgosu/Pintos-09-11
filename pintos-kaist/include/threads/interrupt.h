#ifndef THREADS_INTERRUPT_H
#define THREADS_INTERRUPT_H

#include <stdbool.h>
#include <stdint.h>

/* 인터럽트 켜짐 또는 꺼짐? */
enum intr_level {
	INTR_OFF,             /* 인터럽트 비활성화 */
	INTR_ON               /* 인터럽트 활성화 */
};

enum intr_level intr_get_level (void);
enum intr_level intr_set_level (enum intr_level);
enum intr_level intr_enable (void);
enum intr_level intr_disable (void);

/* 인터럽트 스택 프레임 */
struct gp_registers {
	uint64_t r15;			/* Caller saved*/
	uint64_t r14;			/* Caller saved*/
	uint64_t r13;			/* Caller saved*/
	uint64_t r12;			/* Caller saved*/
	uint64_t r11;			/* Caller saved*/
	uint64_t r10;			/* Caller saved*/
	uint64_t r9;			/* 6th argument	*/
	uint64_t r8;			/* 5th argument */
	uint64_t rsi;			/* 2nd argument: argv */
	uint64_t rdi;			/* 1st argument: argc */
	uint64_t rbp;			/* Callee saved (함수 주소) */
	uint64_t rdx;			/* 3rd argument */
	uint64_t rcx;			/* 4th argument */
	uint64_t rbx;			/* Callee saved (복원 레지스터) */
	uint64_t rax;			/* Return value */
} __attribute__((packed));

struct intr_frame {
	/* intr-stubs.S의 intr_entry에 의해 스택에 푸시됨.
	   인터럽트가 발생한 작업의 저장된 레지스터들 */
	struct gp_registers R;        /* 범용 레지스터들 (rax, rbx, rcx, rdx, rsi, rdi, rbp, r8-r15) */
	uint64_t vec_no;              /* 인터럽트 벡터 번호 */
	uint64_t error_code;          /* 에러 코드 */
	void (*eip) (void);           /* 인터럽트된 코드 주소 */
	uint16_t cs;                  /* 코드 세그먼트 */
	uint16_t pad1;                /* 패딩 */
	uint32_t pad2;                /* 패딩 */
	uint64_t eflags;              /* 저장된 플래그 레지스터 */
	void *esp;                    /* 인터럽트된 스택 포인터 */
	uint16_t ss;                  /* 스택 세그먼트 */
	uint16_t pad3;                /* 패딩 */
	uint32_t pad4;                /* 패딩 */
} __attribute__((packed));

typedef void intr_handler_func (struct intr_frame *);

void intr_init (void);
void intr_register_ext (uint8_t vec, intr_handler_func *, const char *name);
void intr_register_int (uint8_t vec, int dpl, enum intr_level,
                        intr_handler_func *, const char *name);
bool intr_context (void);
void intr_yield_on_return (void);

void intr_dump_frame (const struct intr_frame *);
const char *intr_name (uint8_t vec);

#endif /* threads/interrupt.h */
