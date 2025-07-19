#ifndef FILESYS_OFF_T_H
#define FILESYS_OFF_T_H

#include <stdint.h>

/* 파일 내의 오프셋.
 * 이는 별도의 헤더로 분리되어 있는데, 여러 헤더가 이 정의를 필요로 하지만
 * 다른 정의들은 필요로 하지 않기 때문입니다. */
typedef int32_t off_t;

/* printf()를 위한 형식 지정자, 예:
 * printf ("offset=%"PROTd"\n", offset); */
#define PROTd PRId32

#endif /* filesys/off_t.h */
