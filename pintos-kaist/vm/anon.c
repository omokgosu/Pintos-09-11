/* anon.c: 디스크가 아닌 이미지 페이지(익명 페이지)의 구현. */

#include "vm/vm.h"
#include "devices/disk.h"

/* 아래 줄을 수정하지 마세요 */
static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

/* 이 구조체를 수정하지 마세요 */
static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

/* 익명 페이지의 데이터를 초기화합니다 */
void
vm_anon_init (void) {
	/* TODO: swap_disk를 설정하세요. */
	swap_disk = NULL;
}

/* 파일 매핑을 초기화합니다 */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* 핸들러를 설정합니다 */
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;
}

/* 스왑 디스크에서 내용을 읽어 페이지를 스왑 인합니다. */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;
}

/* 스왑 디스크에 내용을 써서 페이지를 스왑 아웃합니다. */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
}

/* 익명 페이지를 파괴합니다. PAGE는 호출자에 의해 해제됩니다. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page = &page->anon;
}
