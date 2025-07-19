#ifndef DEVICES_INTQ_H
#define DEVICES_INTQ_H

#include "threads/interrupt.h"
#include "threads/synch.h"

/* "인터럽트 큐", 커널 스레드와 외부 인터럽트 핸들러 간에 공유되는 
   원형 버퍼입니다.

   인터럽트 큐 함수들은 커널 스레드나 외부 인터럽트 핸들러에서 
   호출될 수 있습니다. intq_init()을 제외하고는 두 경우 모두 
   인터럽트가 비활성화되어 있어야 합니다.

   인터럽트 큐는 "모니터"의 구조를 가집니다. threads/synch.h의 
   락과 조건 변수는 일반적으로 사용되는 것처럼 이 경우에는 
   사용할 수 없습니다. 왜냐하면 이들은 커널 스레드들 간의 보호만 
   할 수 있고, 인터럽트 핸들러로부터는 보호할 수 없기 때문입니다. */

/* 큐 버퍼 크기(바이트 단위) */
#define INTQ_BUFSIZE 64

/* 바이트의 원형 큐 */
struct intq {
	/* 대기 중인 스레드들 */
	struct lock lock;           /* 한 번에 하나의 스레드만 대기할 수 있음 */
	struct thread *not_full;    /* not-full 조건을 기다리는 스레드 */
	struct thread *not_empty;   /* not-empty 조건을 기다리는 스레드 */

	/* 큐 */
	uint8_t buf[INTQ_BUFSIZE];  /* 버퍼 */
	int head;                   /* 새로운 데이터가 여기에 쓰임 */
	int tail;                   /* 오래된 데이터가 여기서 읽힘 */
};

void intq_init (struct intq *);
bool intq_empty (const struct intq *);
bool intq_full (const struct intq *);
uint8_t intq_getc (struct intq *);
void intq_putc (struct intq *, uint8_t);

#endif /* devices/intq.h */
