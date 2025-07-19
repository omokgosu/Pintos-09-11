#include "devices/vga.h"
#include <round.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "threads/io.h"
#include "threads/interrupt.h"
#include "threads/vaddr.h"

/* VGA 텍스트 화면 지원. 더 많은 정보는 [FREEVGA]를 참조하세요. */

/* 텍스트 디스플레이의 열과 행 수. */
#define COL_CNT 80
#define ROW_CNT 25

/* 현재 커서 위치. (0,0)은 디스플레이의 왼쪽 상단 모서리입니다. */
static size_t cx, cy;

/* 검은 배경에 회색 텍스트의 속성 값. */
#define GRAY_ON_BLACK 0x07

/* 프레임버퍼. "VGA 텍스트 모드 작동" 하에서 [FREEVGA]를 참조하세요.
   (x,y)에 있는 문자는 fb[y][x][0]입니다.
   (x,y)에 있는 속성은 fb[y][x][1]입니다. */
static uint8_t (*fb)[COL_CNT][2];

static void clear_row (size_t y);
static void cls (void);
static void newline (void);
static void move_cursor (void);
static void find_cursor (size_t *x, size_t *y);

/* VGA 텍스트 디스플레이를 초기화합니다. */
static void
init (void) {
	/* 이미 초기화되었나요? */
	static bool inited;
	if (!inited) {
		fb = ptov (0xb8000);
		find_cursor (&cx, &cy);
		inited = true;
	}
}

/* C를 VGA 텍스트 디스플레이에 쓰며, 제어 문자를 
   일반적인 방식으로 해석합니다. */
void
vga_putc (int c) {
	/* 콘솔에 쓸 수 있는 인터럽트 핸들러를 차단하기 위해
	   인터럽트를 비활성화합니다. */
	enum intr_level old_level = intr_disable ();

	init ();

	switch (c) {
		case '\n':
			newline ();
			break;

		case '\f':
			cls ();
			break;

		case '\b':
			if (cx > 0)
				cx--;
			break;

		case '\r':
			cx = 0;
			break;

		case '\t':
			cx = ROUND_UP (cx + 1, 8);
			if (cx >= COL_CNT)
				newline ();
			break;

		default:
			fb[cy][cx][0] = c;
			fb[cy][cx][1] = GRAY_ON_BLACK;
			if (++cx >= COL_CNT)
				newline ();
			break;
	}

	/* 커서 위치를 업데이트합니다. */
	move_cursor ();

	intr_set_level (old_level);
}

/* 화면을 지우고 커서를 왼쪽 상단으로 이동합니다. */
static void
cls (void) {
	size_t y;

	for (y = 0; y < ROW_CNT; y++)
		clear_row (y);

	cx = cy = 0;
	move_cursor ();
}

/* Y 행을 공백으로 지웁니다. */
static void
clear_row (size_t y) {
	size_t x;

	for (x = 0; x < COL_CNT; x++)
	{
		fb[y][x][0] = ' ';
		fb[y][x][1] = GRAY_ON_BLACK;
	}
}

/* 커서를 화면의 다음 줄 첫 번째 열로 이동합니다.
   커서가 이미 화면의 마지막 줄에 있으면,
   화면을 한 줄 위로 스크롤합니다. */
static void
newline (void) {
	cx = 0;
	cy++;
	if (cy >= ROW_CNT)
	{
		cy = ROW_CNT - 1;
		memmove (&fb[0], &fb[1], sizeof fb[0] * (ROW_CNT - 1));
		clear_row (ROW_CNT - 1);
	}
}

/* 하드웨어 커서를 (cx,cy)로 이동합니다. */
static void
move_cursor (void) {
	/* "텍스트 모드 커서 조작" 하에서 [FREEVGA]를 참조하세요. */
	uint16_t cp = cx + COL_CNT * cy;
	outw (0x3d4, 0x0e | (cp & 0xff00));
	outw (0x3d4, 0x0f | (cp << 8));
}

/* 현재 하드웨어 커서 위치를 (*X,*Y)로 읽습니다. */
static void
find_cursor (size_t *x, size_t *y) {
	/* "텍스트 모드 커서 조작" 하에서 [FREEVGA]를 참조하세요. */
	uint16_t cp;

	outb (0x3d4, 0x0e);
	cp = inb (0x3d5) << 8;

	outb (0x3d4, 0x0f);
	cp |= inb (0x3d5);

	*x = cp % COL_CNT;
	*y = cp / COL_CNT;
}
