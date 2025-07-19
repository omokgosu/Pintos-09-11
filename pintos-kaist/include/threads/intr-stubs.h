#ifndef THREADS_INTR_STUBS_H
#define THREADS_INTR_STUBS_H

/* 인터럽트 스텁들
 *
 * 이들은 intr-stubs.S의 작은 코드 조각들로, 가능한 256개의 x86 인터럽트 
 * 각각에 대해 하나씩 있습니다. 각각은 약간의 스택 조작을 수행한 다음 
 * intr_entry()로 점프합니다. 더 많은 정보는 intr-stubs.S를 참조하세요.
 *
 * 이 배열은 intr_init()이 쉽게 찾을 수 있도록 각 인터럽트 스텁 진입점을 
 * 가리킵니다. */
typedef void intr_stub_func (void);
extern intr_stub_func *intr_stubs[256];

#endif /* threads/intr-stubs.h */
