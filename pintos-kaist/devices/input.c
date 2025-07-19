#include "devices/input.h"
#include <debug.h>
#include "devices/intq.h"
#include "devices/serial.h"

/* 키보드와 시리얼 포트에서 키를 저장합니다. */
static struct intq buffer;

/* 입력 버퍼를 초기화합니다. */
void
input_init (void) {
	intq_init (&buffer);
}

/* 입력 버퍼에 키를 추가합니다.
   인터럽트가 꺼져있어야 하고 버퍼가 가득 차있지 않아야 합니다. */
void
input_putc (uint8_t key) {
	ASSERT (intr_get_level () == INTR_OFF);
	ASSERT (!intq_full (&buffer));

	intq_putc (&buffer, key);
	serial_notify ();
}

/* 입력 버퍼에서 키를 가져옵니다.
   버퍼가 비어있으면 키가 눌릴 때까지 기다립니다. */
uint8_t
input_getc (void) {
	enum intr_level old_level;
	uint8_t key;

	old_level = intr_disable ();
	key = intq_getc (&buffer);
	serial_notify ();
	intr_set_level (old_level);

	return key;
}

/* 입력 버퍼가 가득 차있으면 true를 반환하고,
   그렇지 않으면 false를 반환합니다.
   인터럽트가 꺼져있어야 합니다. */
bool
input_full (void) {
	ASSERT (intr_get_level () == INTR_OFF);
	return intq_full (&buffer);
}
