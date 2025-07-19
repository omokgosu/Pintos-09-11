#include "threads/loader.h"

/* Main interrupt entry point.
   메인 인터럽트 진입점

   내부 또는 외부 인터럽트가 발생하면 intrNN_stub 루틴 중 하나에서 시작되며,
   이 루틴들은 `struct intr_frame'의 frame_pointer, error_code, vec_no 멤버들을
   스택에 푸시한 후 여기로 점프합니다.

   나머지 `struct intr_frame' 멤버들을 스택에 저장하고, 커널에 필요한 레지스터들을
   설정한 후, 실제로 인터럽트를 처리하는 intr_handler()를 호출합니다.
*/
.section .text
.func intr_entry
intr_entry:
	/* Save caller's registers. */
	/* 호출자의 레지스터들을 저장합니다. */
	subq $16,%rsp		/* 스택 포인터를 16바이트 감소 (세그먼트 레지스터 저장 공간) */
	movw %ds,8(%rsp)	/* DS 세그먼트 레지스터를 스택에 저장 */
	movw %es,0(%rsp)	/* ES 세그먼트 레지스터를 스택에 저장 */
	subq $120,%rsp		/* 스택 포인터를 120바이트 감소 (범용 레지스터 저장 공간) */
	movq %rax,112(%rsp)	/* RAX 레지스터를 스택에 저장 */
	movq %rbx,104(%rsp)	/* RBX 레지스터를 스택에 저장 */
	movq %rcx,96(%rsp)	/* RCX 레지스터를 스택에 저장 */
	movq %rdx,88(%rsp)	/* RDX 레지스터를 스택에 저장 */
	movq %rbp,80(%rsp)	/* RBP 레지스터를 스택에 저장 */
	movq %rdi,72(%rsp)	/* RDI 레지스터를 스택에 저장 */
	movq %rsi,64(%rsp)	/* RSI 레지스터를 스택에 저장 */
	movq %r8,56(%rsp)	/* R8 레지스터를 스택에 저장 */
	movq %r9,48(%rsp)	/* R9 레지스터를 스택에 저장 */
	movq %r10,40(%rsp)	/* R10 레지스터를 스택에 저장 */
	movq %r11,32(%rsp)	/* R11 레지스터를 스택에 저장 */
	movq %r12,24(%rsp)	/* R12 레지스터를 스택에 저장 */
	movq %r13,16(%rsp)	/* R13 레지스터를 스택에 저장 */
	movq %r14,8(%rsp)	/* R14 레지스터를 스택에 저장 */
	movq %r15,0(%rsp)	/* R15 레지스터를 스택에 저장 */
	cld			/* String instructions go upward. */
				/* 방향 플래그 클리어 (문자열 명령어가 위쪽으로 진행) */
	movq $SEL_KDSEG, %rax	/* 커널 데이터 세그먼트 셀렉터를 RAX에 로드 */
	movw %ax, %ds		/* DS 세그먼트 레지스터를 커널 데이터 세그먼트로 설정 */
	movw %ax, %es		/* ES 세그먼트 레지스터를 커널 데이터 세그먼트로 설정 */
	movw %ax, %ss		/* SS 세그먼트 레지스터를 커널 데이터 세그먼트로 설정 */
	movw %ax, %fs		/* FS 세그먼트 레지스터를 커널 데이터 세그먼트로 설정 */
	movw %ax, %gs		/* GS 세그먼트 레지스터를 커널 데이터 세그먼트로 설정 */
	movq %rsp,%rdi		/* 스택 포인터를 RDI에 복사 (intr_handler의 첫 번째 인자) */
	call intr_handler	/* 인터럽트 핸들러 호출 */
	
	/* 레지스터 복원 (저장의 역순) */
	movq 0(%rsp), %r15	/* R15 레지스터 복원 */
	movq 8(%rsp), %r14	/* R14 레지스터 복원 */
	movq 16(%rsp), %r13	/* R13 레지스터 복원 */
	movq 24(%rsp), %r12	/* R12 레지스터 복원 */
	movq 32(%rsp), %r11	/* R11 레지스터 복원 */
	movq 40(%rsp), %r10	/* R10 레지스터 복원 */
	movq 48(%rsp), %r9	/* R9 레지스터 복원 */
	movq 56(%rsp), %r8	/* R8 레지스터 복원 */
	movq 64(%rsp), %rsi	/* RSI 레지스터 복원 */
	movq 72(%rsp), %rdi	/* RDI 레지스터 복원 */
	movq 80(%rsp), %rbp	/* RBP 레지스터 복원 */
	movq 88(%rsp), %rdx	/* RDX 레지스터 복원 */
	movq 96(%rsp), %rcx	/* RCX 레지스터 복원 */
	movq 104(%rsp), %rbx	/* RBX 레지스터 복원 */
	movq 112(%rsp), %rax	/* RAX 레지스터 복원 */
	addq $120, %rsp		/* 스택 포인터를 120바이트 증가 (범용 레지스터 공간 해제) */
	movw 8(%rsp), %ds	/* DS 세그먼트 레지스터 복원 */
	movw (%rsp), %es	/* ES 세그먼트 레지스터 복원 */
	addq $32, %rsp		/* 스택 포인터를 32바이트 증가 (최종 정리) */
	iretq			/* 인터럽트에서 복귀 (RIP, CS, RFLAGS, RSP, SS 복원) */
.endfunc

/* Interrupt stubs.
   인터럽트 스텁들

   이것은 `intr00_stub'부터 `intrff_stub'까지 이름이 붙은 256개의 코드 조각들을
   정의하며, 각각은 해당 인터럽트 벡터의 진입점으로 사용됩니다. 또한 이 함수들의
   주소를 함수 포인터 배열인 `intr_stubs'의 올바른 위치에 배치합니다.

   대부분의 스텁들은 다음과 같이 동작합니다:

        1. %ebp를 스택에 푸시 (`struct intr_frame'의 frame_pointer)

        2. 0을 스택에 푸시 (error_code)

        3. 인터럽트 번호를 스택에 푸시 (vec_no)

   CPU는 몇몇 인터럽트에 대해 추가적인 "에러 코드"를 스택에 푸시합니다.
   %ebp가 에러 코드가 있는 위치에 오기를 원하므로, 다른 경로를 따릅니다:

        1. 에러 코드의 복사본을 스택에 푸시

        2. 원본 에러 코드를 %ebp로 교체

        3. 인터럽트 번호를 스택에 푸시 */

/* 위에서 설명한 1단계와 2단계를 구현합니다. 단순히 0 에러 코드를 푸시하는 일반적인 경우입니다. */
#define zero pushq $0;

/* 위에서 설명한 1단계와 2단계를 구현합니다. CPU가 이미 에러 코드를 푸시한 경우입니다. */
#define REAL

.section .data
.globl intr_stubs
intr_stubs:

/* 인터럽트 벡터 NUMBER에 대한 스텁을 생성합니다.
   TYPE은 `zero' (0 에러 코드를 푸시하는 경우) 또는
   `REAL' (CPU가 에러 코드를 푸시하는 경우)입니다. */
#define STUB(NUMBER, TYPE)                      \
.section .text;                                  \
.globl intr##NUMBER##_stub;                     \
.func intr##NUMBER##_stub;			\
intr##NUMBER##_stub:                            \
	TYPE;                                   \
	push $0x##NUMBER;                       \
	jmp intr_entry;                         \
.endfunc; \
.section .data; \
.quad intr##NUMBER##_stub;

/* All the stubs. */
/* 모든 스텁들 */
STUB(00, zero) STUB(01, zero) STUB(02, zero) STUB(03, zero)
STUB(04, zero) STUB(05, zero) STUB(06, zero) STUB(07, zero)
STUB(08, REAL) STUB(09, zero) STUB(0a, REAL) STUB(0b, REAL)
STUB(0c, zero) STUB(0d, REAL) STUB(0e, REAL) STUB(0f, zero)

STUB(10, zero) STUB(11, REAL) STUB(12, zero) STUB(13, zero)
STUB(14, zero) STUB(15, zero) STUB(16, zero) STUB(17, zero)
STUB(18, REAL) STUB(19, zero) STUB(1a, REAL) STUB(1b, REAL)
STUB(1c, zero) STUB(1d, REAL) STUB(1e, REAL) STUB(1f, zero)

STUB(20, zero) STUB(21, zero) STUB(22, zero) STUB(23, zero)
STUB(24, zero) STUB(25, zero) STUB(26, zero) STUB(27, zero)
STUB(28, zero) STUB(29, zero) STUB(2a, zero) STUB(2b, zero)
STUB(2c, zero) STUB(2d, zero) STUB(2e, zero) STUB(2f, zero)

STUB(30, zero) STUB(31, zero) STUB(32, zero) STUB(33, zero)
STUB(34, zero) STUB(35, zero) STUB(36, zero) STUB(37, zero)
STUB(38, zero) STUB(39, zero) STUB(3a, zero) STUB(3b, zero)
STUB(3c, zero) STUB(3d, zero) STUB(3e, zero) STUB(3f, zero)

STUB(40, zero) STUB(41, zero) STUB(42, zero) STUB(43, zero)
STUB(44, zero) STUB(45, zero) STUB(46, zero) STUB(47, zero)
STUB(48, zero) STUB(49, zero) STUB(4a, zero) STUB(4b, zero)
STUB(4c, zero) STUB(4d, zero) STUB(4e, zero) STUB(4f, zero)

STUB(50, zero) STUB(51, zero) STUB(52, zero) STUB(53, zero)
STUB(54, zero) STUB(55, zero) STUB(56, zero) STUB(57, zero)
STUB(58, zero) STUB(59, zero) STUB(5a, zero) STUB(5b, zero)
STUB(5c, zero) STUB(5d, zero) STUB(5e, zero) STUB(5f, zero)

STUB(60, zero) STUB(61, zero) STUB(62, zero) STUB(63, zero)
STUB(64, zero) STUB(65, zero) STUB(66, zero) STUB(67, zero)
STUB(68, zero) STUB(69, zero) STUB(6a, zero) STUB(6b, zero)
STUB(6c, zero) STUB(6d, zero) STUB(6e, zero) STUB(6f, zero)

STUB(70, zero) STUB(71, zero) STUB(72, zero) STUB(73, zero)
STUB(74, zero) STUB(75, zero) STUB(76, zero) STUB(77, zero)
STUB(78, zero) STUB(79, zero) STUB(7a, zero) STUB(7b, zero)
STUB(7c, zero) STUB(7d, zero) STUB(7e, zero) STUB(7f, zero)

STUB(80, zero) STUB(81, zero) STUB(82, zero) STUB(83, zero)
STUB(84, zero) STUB(85, zero) STUB(86, zero) STUB(87, zero)
STUB(88, zero) STUB(89, zero) STUB(8a, zero) STUB(8b, zero)
STUB(8c, zero) STUB(8d, zero) STUB(8e, zero) STUB(8f, zero)

STUB(90, zero) STUB(91, zero) STUB(92, zero) STUB(93, zero)
STUB(94, zero) STUB(95, zero) STUB(96, zero) STUB(97, zero)
STUB(98, zero) STUB(99, zero) STUB(9a, zero) STUB(9b, zero)
STUB(9c, zero) STUB(9d, zero) STUB(9e, zero) STUB(9f, zero)

STUB(a0, zero) STUB(a1, zero) STUB(a2, zero) STUB(a3, zero)
STUB(a4, zero) STUB(a5, zero) STUB(a6, zero) STUB(a7, zero)
STUB(a8, zero) STUB(a9, zero) STUB(aa, zero) STUB(ab, zero)
STUB(ac, zero) STUB(ad, zero) STUB(ae, zero) STUB(af, zero)

STUB(b0, zero) STUB(b1, zero) STUB(b2, zero) STUB(b3, zero)
STUB(b4, zero) STUB(b5, zero) STUB(b6, zero) STUB(b7, zero)
STUB(b8, zero) STUB(b9, zero) STUB(ba, zero) STUB(bb, zero)
STUB(bc, zero) STUB(bd, zero) STUB(be, zero) STUB(bf, zero)

STUB(c0, zero) STUB(c1, zero) STUB(c2, zero) STUB(c3, zero)
STUB(c4, zero) STUB(c5, zero) STUB(c6, zero) STUB(c7, zero)
STUB(c8, zero) STUB(c9, zero) STUB(ca, zero) STUB(cb, zero)
STUB(cc, zero) STUB(cd, zero) STUB(ce, zero) STUB(cf, zero)

STUB(d0, zero) STUB(d1, zero) STUB(d2, zero) STUB(d3, zero)
STUB(d4, zero) STUB(d5, zero) STUB(d6, zero) STUB(d7, zero)
STUB(d8, zero) STUB(d9, zero) STUB(da, zero) STUB(db, zero)
STUB(dc, zero) STUB(dd, zero) STUB(de, zero) STUB(df, zero)

STUB(e0, zero) STUB(e1, zero) STUB(e2, zero) STUB(e3, zero)
STUB(e4, zero) STUB(e5, zero) STUB(e6, zero) STUB(e7, zero)
STUB(e8, zero) STUB(e9, zero) STUB(ea, zero) STUB(eb, zero)
STUB(ec, zero) STUB(ed, zero) STUB(ee, zero) STUB(ef, zero)

STUB(f0, zero) STUB(f1, zero) STUB(f2, zero) STUB(f3, zero)
STUB(f4, zero) STUB(f5, zero) STUB(f6, zero) STUB(f7, zero)
STUB(f8, zero) STUB(f9, zero) STUB(fa, zero) STUB(fb, zero)
STUB(fc, zero) STUB(fd, zero) STUB(fe, zero) STUB(ff, zero)
