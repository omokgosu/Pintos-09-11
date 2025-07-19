/* page_cache.c: 페이지 캐시(버퍼 캐시) 구현 */

#include "vm/vm.h"
static bool page_cache_readahead (struct page *page, void *kva);
static bool page_cache_writeback (struct page *page);
static void page_cache_destroy (struct page *page);

/* 이 구조체를 수정하지 마세요 */
static const struct page_operations page_cache_op = {
	.swap_in = page_cache_readahead,
	.swap_out = page_cache_writeback,
	.destroy = page_cache_destroy,
	.type = VM_PAGE_CACHE,
};

tid_t page_cache_workerd;

/* 파일 vm의 초기화 함수 */
void
pagecache_init (void) {
	/* TODO: page_cache_kworkerd를 사용하여 페이지 캐시용 워커 데몬을 생성하세요 */
}

/* 페이지 캐시 초기화 */
bool
page_cache_initializer (struct page *page, enum vm_type type, void *kva) {
	/* 핸들러 설정 */
	page->operations = &page_cache_op;

}

/* 스왑 인 메커니즘을 활용하여 미리 읽기 구현 */
static bool
page_cache_readahead (struct page *page, void *kva) {
}

/* 스왑 아웃 메커니즘을 활용하여 쓰기 후 저장 구현 */
static bool
page_cache_writeback (struct page *page) {
}

/* page_cache를 파괴합니다 */
static void
page_cache_destroy (struct page *page) {
}

/* 페이지 캐시용 워커 스레드 */
static void
page_cache_kworkerd (void *aux) {
}
