#include "devices/serial.h"
#include <debug.h>
#include "devices/input.h"
#include "devices/intq.h"
#include "devices/timer.h"
#include "threads/io.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"

/* PC에서 사용되는 16550A UART에 대한 레지스터 정의.
   16550A는 여기서 보여준 것보다 훨씬 많은 기능을 가지고 있지만,
   이것이 우리가 필요한 전부입니다.

   하드웨어 정보는 [PC16650D]를 참조하세요. */

/* 첫 번째 시리얼 포트의 I/O 포트 기본 주소. */
#define IO_BASE 0x3f8

/* DLAB=0 레지스터. */
#define RBR_REG (IO_BASE + 0)   /* 수신 버퍼 레지스터 (읽기 전용). */
#define THR_REG (IO_BASE + 0)   /* 송신 홀딩 레지스터 (쓰기 전용). */
#define IER_REG (IO_BASE + 1)   /* 인터럽트 활성화 레지스터. */

/* DLAB=1 레지스터. */
#define LS_REG (IO_BASE + 0)    /* 분할기 래치 (LSB). */
#define MS_REG (IO_BASE + 1)    /* 분할기 래치 (MSB). */

/* DLAB에 무관한 레지스터. */
#define IIR_REG (IO_BASE + 2)   /* 인터럽트 식별 레지스터 (읽기 전용) */
#define FCR_REG (IO_BASE + 2)   /* FIFO 제어 레지스터 (쓰기 전용). */
#define LCR_REG (IO_BASE + 3)   /* 라인 제어 레지스터. */
#define MCR_REG (IO_BASE + 4)   /* 모뎀 제어 레지스터. */
#define LSR_REG (IO_BASE + 5)   /* 라인 상태 레지스터 (읽기 전용). */

/* 인터럽트 활성화 레지스터 비트. */
#define IER_RECV 0x01           /* 데이터 수신 시 인터럽트. */
#define IER_XMIT 0x02           /* 송신 완료 시 인터럽트. */

/* 라인 제어 레지스터 비트. */
#define LCR_N81 0x03            /* 패리티 없음, 8 데이터 비트, 1 정지 비트. */
#define LCR_DLAB 0x80           /* 분할기 래치 액세스 비트 (DLAB). */

/* 모뎀 제어 레지스터. */
#define MCR_OUT2 0x08           /* 출력 라인 2. */

/* 라인 상태 레지스터. */
#define LSR_DR 0x01             /* 데이터 준비: 수신된 데이터 바이트가 RBR에 있음. */
#define LSR_THRE 0x20           /* THR 비어있음. */

/* 송신 모드. */
static enum { UNINIT, POLL, QUEUE } mode;

/* 송신될 데이터. */
static struct intq txq;

static void set_serial (int bps);
static void putc_poll (uint8_t);
static void write_ier (void);
static intr_handler_func serial_interrupt;

/* 폴링 모드로 시리얼 포트 장치를 초기화합니다.
   폴링 모드는 시리얼 포트가 사용 가능해질 때까지 바쁘게 기다린 후
   쓰기를 수행합니다. 느리지만, 인터럽트가 초기화되기 전까지는
   이것이 우리가 할 수 있는 전부입니다. */
static void
init_poll (void) {
	ASSERT (mode == UNINIT);
	outb (IER_REG, 0);                    /* 모든 인터럽트를 끕니다. */
	outb (FCR_REG, 0);                    /* FIFO를 비활성화합니다. */
	set_serial (115200);                  /* 115.2 kbps, N-8-1. */
	outb (MCR_REG, MCR_OUT2);             /* 인터럽트 활성화에 필요합니다. */
	intq_init (&txq);
	mode = POLL;
}

/* 큐 기반 인터럽트 구동 I/O로 시리얼 포트 장치를 초기화합니다.
   인터럽트 구동 I/O를 사용하면 시리얼 장치가 준비될 때까지 기다리는 데
   CPU 시간을 낭비하지 않습니다. */
void
serial_init_queue (void) {
	enum intr_level old_level;

	if (mode == UNINIT)
		init_poll ();
	ASSERT (mode == POLL);

	intr_register_ext (0x20 + 4, serial_interrupt, "serial");
	mode = QUEUE;
	old_level = intr_disable ();
	write_ier ();
	intr_set_level (old_level);
}

/* BYTE를 시리얼 포트로 전송합니다. */
void
serial_putc (uint8_t byte) {
	enum intr_level old_level = intr_disable ();

	if (mode != QUEUE) {
		/* 아직 인터럽트 구동 I/O로 설정되지 않았다면,
		   단순한 폴링을 사용하여 바이트를 전송합니다. */
		if (mode == UNINIT)
			init_poll ();
		putc_poll (byte);
	} else {
		/* 그렇지 않으면, 바이트를 큐에 넣고 인터럽트 활성화
		   레지스터를 업데이트합니다. */
		if (old_level == INTR_OFF && intq_full (&txq)) {
			/* 인터럽트가 꺼져있고 송신 큐가 가득 찼습니다.
			   큐가 비워질 때까지 기다리려면
			   인터럽트를 다시 활성화해야 합니다.
			   그것은 예의에 어긋나므로, 대신 폴링을 통해
			   문자를 전송하겠습니다. */
			putc_poll (intq_getc (&txq));
		}

		intq_putc (&txq, byte);
		write_ier ();
	}

	intr_set_level (old_level);
}

/* 시리얼 버퍼에 있는 모든 것을 폴링 모드로 포트에 플러시합니다. */
void
serial_flush (void) {
	enum intr_level old_level = intr_disable ();
	while (!intq_empty (&txq))
		putc_poll (intq_getc (&txq));
	intr_set_level (old_level);
}

/* 입력 버퍼의 충만도가 변경되었을 수 있습니다. 수신 인터럽트를
   차단해야 하는지 재평가합니다.
   문자가 버퍼에 추가되거나 제거될 때 입력 버퍼 루틴에 의해
   호출됩니다. */
void
serial_notify (void) {
	ASSERT (intr_get_level () == INTR_OFF);
	if (mode == QUEUE)
		write_ier ();
}

/* BPS 비트/초로 시리얼 포트를 구성합니다. */
static void
set_serial (int bps) {
	int base_rate = 1843200 / 16;         /* 16550A의 기본 속도, Hz 단위. */
	uint16_t divisor = base_rate / bps;   /* 클록 속도 분할기. */

	ASSERT (bps >= 300 && bps <= 115200);

	/* DLAB를 활성화합니다. */
	outb (LCR_REG, LCR_N81 | LCR_DLAB);

	/* 데이터 속도를 설정합니다. */
	outb (LS_REG, divisor & 0xff);
	outb (MS_REG, divisor >> 8);

	/* DLAB를 재설정합니다. */
	outb (LCR_REG, LCR_N81);
}

/* 인터럽트 활성화 레지스터를 업데이트합니다. */
static void
write_ier (void) {
	uint8_t ier = 0;

	ASSERT (intr_get_level () == INTR_OFF);

	/* 송신할 문자가 있으면 송신 인터럽트를 활성화합니다. */
	if (!intq_empty (&txq))
		ier |= IER_XMIT;

	/* 수신한 문자를 저장할 공간이 있으면 수신 인터럽트를 활성화합니다. */
	if (!input_full ())
		ier |= IER_RECV;

	outb (IER_REG, ier);
}

/* 시리얼 포트가 준비될 때까지 폴링한 후 BYTE를 전송합니다. */
static void
putc_poll (uint8_t byte) {
	ASSERT (intr_get_level () == INTR_OFF);

	while ((inb (LSR_REG) & LSR_THRE) == 0)
		continue;
	outb (THR_REG, byte);
}

/* 시리얼 인터럽트 핸들러. */
static void
serial_interrupt (struct intr_frame *f UNUSED) {
	/* UART의 인터럽트에 대해 문의합니다. 이것이 없으면
	   QEMU에서 실행할 때 가끔 인터럽트를 놓칠 수 있습니다. */
	inb (IIR_REG);

	/* 바이트를 수신할 공간이 있고 하드웨어가 우리를 위한 바이트를 가지고 있는 한
	   바이트를 수신합니다. */
	while (!input_full () && (inb (LSR_REG) & LSR_DR) != 0)
		input_putc (inb (RBR_REG));

	/* 송신할 바이트가 있고 하드웨어가 송신을 위한 바이트를 받을 준비가 된 한
	   바이트를 송신합니다. */
	while (!intq_empty (&txq) && (inb (LSR_REG) & LSR_THRE) != 0)
		outb (THR_REG, intq_getc (&txq));

	/* 큐 상태에 따라 인터럽트 활성화 레지스터를 업데이트합니다. */
	write_ier ();
}
