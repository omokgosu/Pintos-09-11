#include "devices/kbd.h"
#include <ctype.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "devices/input.h"
#include "threads/interrupt.h"
#include "threads/io.h"

/* 키보드 데이터 레지스터 포트. */
#define DATA_REG 0x60

/* shift 키들의 현재 상태.
   눌려있으면 true, 그렇지 않으면 false. */
static bool left_shift, right_shift;    /* 왼쪽과 오른쪽 Shift 키. */
static bool left_alt, right_alt;        /* 왼쪽과 오른쪽 Alt 키. */
static bool left_ctrl, right_ctrl;      /* 왼쪽과 오른쪽 Ctrl 키. */

/* Caps Lock의 상태.
   켜져있으면 true, 꺼져있으면 false. */
static bool caps_lock;

/* 눌린 키의 개수. */
static int64_t key_cnt;

static intr_handler_func keyboard_interrupt;

/* 키보드를 초기화합니다. */
void
kbd_init (void) {
	intr_register_ext (0x21, keyboard_interrupt, "8042 Keyboard");
}

/* 키보드 통계를 출력합니다. */
void
kbd_print_stats (void) {
	printf ("Keyboard: %lld keys pressed\n", key_cnt);
}

/* 연속된 스캔코드 집합을 문자로 매핑합니다. */
struct keymap {
	uint8_t first_scancode;     /* 첫 번째 스캔코드. */
	const char *chars;          /* chars[0]은 first_scancode 스캔코드를 가지고,
								   chars[1]은 first_scancode + 1 스캔코드를 가지며,
								   문자열의 끝까지 이런 식으로 계속됩니다. */
};

/* Shift 키가 눌렸는지 여부에 관계없이 같은 문자를 생성하는 키들.
   문자의 대소문자는 다른 곳에서 처리하는 예외입니다. */
static const struct keymap invariant_keymap[] = {
	{0x01, "\033"},
	{0x0e, "\b"},
	{0x0f, "\tQWERTYUIOP"},
	{0x1c, "\r"},
	{0x1e, "ASDFGHJKL"},
	{0x2c, "ZXCVBNM"},
	{0x37, "*"},
	{0x39, " "},
	{0, NULL},
};

/* Shift 없이 눌린 키들의 문자들,
   중요한 키들에 대해서만. */
static const struct keymap unshifted_keymap[] = {
	{0x02, "1234567890-="},
	{0x1a, "[]"},
	{0x27, ";'`"},
	{0x2b, "\\"},
	{0x33, ",./"},
	{0, NULL},
};

/* Shift와 함께 눌린 키들의 문자들,
   중요한 키들에 대해서만. */
static const struct keymap shifted_keymap[] = {
	{0x02, "!@#$%^&*()_+"},
	{0x1a, "{}"},
	{0x27, ":\"~"},
	{0x2b, "|"},
	{0x33, "<>?"},
	{0, NULL},
};

static bool map_key (const struct keymap[], unsigned scancode, uint8_t *);

static void
keyboard_interrupt (struct intr_frame *args UNUSED) {
	/* shift 키들의 상태. */
	bool shift = left_shift || right_shift;
	bool alt = left_alt || right_alt;
	bool ctrl = left_ctrl || right_ctrl;

	/* 키보드 스캔코드. */
	unsigned code;

	/* 키가 눌린 경우 false, 키가 떼어진 경우 true. */
	bool release;

	/* `code'에 해당하는 문자. */
	uint8_t c;

	/* 접두사 코드가 있는 경우 두 번째 바이트를 포함하여 스캔코드를 읽습니다. */
	code = inb (DATA_REG);
	if (code == 0xe0)
		code = (code << 8) | inb (DATA_REG);

	/* 비트 0x80은 키 누름과 키 떼기를 구분합니다
	   (접두사가 있어도). */
	release = (code & 0x80) != 0;
	code &= ~0x80u;

	/* 키를 해석합니다. */
	if (code == 0x3a) {
		/* Caps Lock. */
		if (!release)
			caps_lock = !caps_lock;
	} else if (map_key (invariant_keymap, code, &c)
			|| (!shift && map_key (unshifted_keymap, code, &c))
			|| (shift && map_key (shifted_keymap, code, &c))) {
		/* 일반 문자. */
		if (!release) {
			/* Ctrl, Shift를 처리합니다.
			   Ctrl이 Shift를 오버라이드한다는 점에 주의하세요. */
			if (ctrl && c >= 0x40 && c < 0x60) {
				/* A는 0x41, Ctrl+A는 0x01, 등등. */
				c -= 0x40;
			} else if (shift == caps_lock)
				c = tolower (c);

			/* 높은 비트를 설정하여 Alt를 처리합니다.
			   이 0x80은 키 누름과 키 떼기를 구분하는 데 사용되는 것과는
			   관련이 없습니다. */
			if (alt)
				c += 0x80;

			/* 키보드 버퍼에 추가합니다. */
			if (!input_full ()) {
				key_cnt++;
				input_putc (c);
			}
		}
	} else {
		/* 키코드를 shift 상태 변수로 매핑합니다. */
		struct shift_key {
			unsigned scancode;
			bool *state_var;
		};

		/* shift 키들의 테이블. */
		static const struct shift_key shift_keys[] = {
			{  0x2a, &left_shift},
			{  0x36, &right_shift},
			{  0x38, &left_alt},
			{0xe038, &right_alt},
			{  0x1d, &left_ctrl},
			{0xe01d, &right_ctrl},
			{0,      NULL},
		};

		const struct shift_key *key;

		/* 테이블을 스캔합니다. */
		for (key = shift_keys; key->scancode != 0; key++)
			if (key->scancode == code) {
				*key->state_var = !release;
				break;
			}
	}
}

/* 키맵 배열 K에서 SCANCODE를 스캔합니다.
   찾으면 *C를 해당 문자로 설정하고 true를 반환합니다.
   찾지 못하면 false를 반환하고 C는 무시됩니다. */
static bool
map_key (const struct keymap k[], unsigned scancode, uint8_t *c) {
	for (; k->first_scancode != 0; k++)
		if (scancode >= k->first_scancode
				&& scancode < k->first_scancode + strlen (k->chars)) {
			*c = k->chars[scancode - k->first_scancode];
			return true;
		}

	return false;
}
