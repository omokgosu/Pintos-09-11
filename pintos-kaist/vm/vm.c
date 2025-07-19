/* vm.c: 가상 메모리 객체를 위한 일반적인 인터페이스. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/* 각 하위 시스템의 초기화 코드를 호출하여 가상 메모리 하위 시스템을 초기화합니다. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* 위의 줄들을 수정하지 마세요. */
	/* TODO: 여기에 코드를 작성하세요. */
}

/* 페이지의 타입을 가져옵니다. 이 함수는 페이지가 초기화된 후의 타입을 알고 싶을 때 유용합니다.
 * 이 함수는 현재 완전히 구현되어 있습니다. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* 도우미 함수들 */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* 초기화자와 함께 대기 중인 페이지 객체를 생성합니다. 페이지를 생성하고 싶다면,
 * 직접 생성하지 말고 이 함수나 `vm_alloc_page`를 통해 만드세요. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* upage가 이미 사용 중인지 확인합니다. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: 페이지를 생성하고, VM 타입에 따라 초기화자를 가져와서
		 * TODO: uninit_new를 호출하여 "uninit" 페이지 구조체를 생성하세요.
		 * TODO: uninit_new를 호출한 후 필드를 수정해야 합니다. */

		/* TODO: 페이지를 spt에 삽입하세요. */
	}
err:
	return false;
}

/* spt에서 VA를 찾아 페이지를 반환합니다. 오류 시 NULL을 반환합니다. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: 이 함수를 채우세요. */

	return page;
}

/* 검증과 함께 PAGE를 spt에 삽입합니다. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: 이 함수를 채우세요. */

	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* 축출될 struct frame을 가져옵니다. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: 축출 정책은 여러분이 정하세요. */

	return victim;
}

/* 하나의 페이지를 축출하고 해당 프레임을 반환합니다.
 * 오류 시 NULL을 반환합니다. */
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: victim을 스왑 아웃하고 축출된 프레임을 반환하세요. */

	return NULL;
}

/* palloc()을 호출하여 프레임을 가져옵니다. 사용 가능한 페이지가 없으면 페이지를 축출하고
 * 반환합니다. 이 함수는 항상 유효한 주소를 반환합니다. 즉, 사용자 풀 메모리가 가득 찬 경우
 * 이 함수는 사용 가능한 메모리 공간을 얻기 위해 프레임을 축출합니다. */
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: 이 함수를 채우세요. */

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* 스택을 증가시킵니다. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* 쓰기 보호된 페이지의 폴트를 처리합니다 */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* 성공 시 true를 반환합니다 */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: 폴트를 검증하세요 */
	/* TODO: 여기에 코드를 작성하세요 */

	return vm_do_claim_page (page);
}

/* 페이지를 해제합니다.
 * 이 함수를 수정하지 마세요. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* VA에 할당된 페이지를 클레임합니다. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: 이 함수를 채우세요 */

	return vm_do_claim_page (page);
}

/* PAGE를 클레임하고 mmu를 설정합니다. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* 링크 설정 */
	frame->page = page;
	page->frame = frame;

	/* TODO: 페이지의 VA를 프레임의 PA에 매핑하는 페이지 테이블 항목을 삽입하세요. */

	return swap_in (page, frame->kva);
}

/* 새로운 보조 페이지 테이블을 초기화합니다 */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
}

/* 보조 페이지 테이블을 src에서 dst로 복사합니다 */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* 보조 페이지 테이블이 보유한 리소스를 해제합니다 */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: 스레드가 보유한 모든 supplemental_page_table을 파괴하고
	 * TODO: 수정된 모든 내용을 저장소에 라이트백하세요. */
}
