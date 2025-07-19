#ifndef THREADS_INIT_H
#define THREADS_INIT_H

#include <debug.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* 물리 메모리 크기, 4KB 페이지 단위 */
extern size_t ram_pages;

/* 커널 매핑만 있는 페이지 맵 레벨 4 */
extern uint64_t *base_pml4;

/* -q: 커널 태스크 완료 시 전원 종료? */
extern bool power_off_when_done;

void power_off (void) NO_RETURN;

#endif /* threads/init.h */
