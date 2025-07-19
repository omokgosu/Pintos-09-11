#ifndef __LIB_KERNEL_HASH_H
#define __LIB_KERNEL_HASH_H

/* 해시 테이블
 *
 * 이 데이터 구조는 프로젝트 3용 Pintos 투어에서 철저히 문서화되어 있습니다.
 *
 * 이것은 체이닝을 사용하는 표준 해시 테이블입니다. 테이블에서 요소를 찾기 위해, 
 * 요소의 데이터에 대해 해시 함수를 계산하고 이를 이중 연결 리스트 배열의 
 * 인덱스로 사용한 다음, 리스트를 선형 검색합니다.
 *
 * 체인 리스트는 동적 할당을 사용하지 않습니다. 대신, 해시에 잠재적으로 포함될 수 
 * 있는 각 구조체는 struct hash_elem 멤버를 포함해야 합니다. 모든 해시 함수들은 
 * 이러한 `struct hash_elem`들에 대해 작동합니다. hash_entry 매크로는 
 * struct hash_elem에서 그것을 포함하는 구조체 객체로의 변환을 허용합니다. 
 * 이것은 연결 리스트 구현에서 사용되는 것과 같은 기법입니다. 
 * 자세한 설명은 lib/kernel/list.h를 참조하세요. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "list.h"

/* 해시 요소 */
struct hash_elem {
	struct list_elem list_elem;
};

/* 해시 요소 HASH_ELEM에 대한 포인터를 HASH_ELEM이 포함된 구조체에 대한 
 * 포인터로 변환합니다. 외부 구조체 STRUCT의 이름과 해시 요소의 멤버 이름 
 * MEMBER를 제공하세요. 예제는 파일 상단의 큰 주석을 참조하세요. */
#define hash_entry(HASH_ELEM, STRUCT, MEMBER)                   \
	((STRUCT *) ((uint8_t *) &(HASH_ELEM)->list_elem        \
		- offsetof (STRUCT, MEMBER.list_elem)))

/* 보조 데이터 AUX가 주어진 상태에서 해시 요소 E의 해시 값을 계산하고 반환합니다. */
typedef uint64_t hash_hash_func (const struct hash_elem *e, void *aux);

/* 보조 데이터 AUX가 주어진 상태에서 두 해시 요소 A와 B의 값을 비교합니다. 
 * A가 B보다 작으면 true를, A가 B보다 크거나 같으면 false를 반환합니다. */
typedef bool hash_less_func (const struct hash_elem *a,
		const struct hash_elem *b,
		void *aux);

/* 보조 데이터 AUX가 주어진 상태에서 해시 요소 E에 대해 어떤 연산을 수행합니다. */
typedef void hash_action_func (struct hash_elem *e, void *aux);

/* 해시 테이블 */
struct hash {
	size_t elem_cnt;            /* 테이블의 요소 수 */
	size_t bucket_cnt;          /* 버킷 수, 2의 거듭제곱 */
	struct list *buckets;       /* `bucket_cnt`개의 리스트 배열 */
	hash_hash_func *hash;       /* 해시 함수 */
	hash_less_func *less;       /* 비교 함수 */
	void *aux;                  /* `hash`와 `less`를 위한 보조 데이터 */
};

/* 해시 테이블 반복자 */
struct hash_iterator {
	struct hash *hash;          /* 해시 테이블 */
	struct list *bucket;        /* 현재 버킷 */
	struct hash_elem *elem;     /* 현재 버킷의 현재 해시 요소 */
};

/* 기본 생명주기 */
bool hash_init (struct hash *, hash_hash_func *, hash_less_func *, void *aux);
void hash_clear (struct hash *, hash_action_func *);
void hash_destroy (struct hash *, hash_action_func *);

/* 검색, 삽입, 삭제 */
struct hash_elem *hash_insert (struct hash *, struct hash_elem *);
struct hash_elem *hash_replace (struct hash *, struct hash_elem *);
struct hash_elem *hash_find (struct hash *, struct hash_elem *);
struct hash_elem *hash_delete (struct hash *, struct hash_elem *);

/* 반복 */
void hash_apply (struct hash *, hash_action_func *);
void hash_first (struct hash_iterator *, struct hash *);
struct hash_elem *hash_next (struct hash_iterator *);
struct hash_elem *hash_cur (struct hash_iterator *);

/* 정보 */
size_t hash_size (struct hash *);
bool hash_empty (struct hash *);

/* 샘플 해시 함수들 */
uint64_t hash_bytes (const void *, size_t);
uint64_t hash_string (const char *);
uint64_t hash_int (int);

#endif /* lib/kernel/hash.h */
