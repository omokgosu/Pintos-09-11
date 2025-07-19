#include "threads/init.h"
#include <console.h>
#include <debug.h>
#include <limits.h>
#include <random.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "devices/kbd.h"
#include "devices/input.h"
#include "devices/serial.h"
#include "devices/timer.h"
#include "devices/vga.h"
#include "threads/interrupt.h"
#include "threads/io.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/mmu.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/thread.h"
#ifdef USERPROG
#include "userprog/process.h"
#include "userprog/exception.h"
#include "userprog/gdt.h"
#include "userprog/syscall.h"
#include "userprog/tss.h"
#endif
#include "tests/threads/tests.h"
#ifdef VM
#include "vm/vm.h"
#endif
#ifdef FILESYS
#include "devices/disk.h"
#include "filesys/filesys.h"
#include "filesys/fsutil.h"
#endif

/* 커널 매핑만 포함하는 페이지 맵 레벨 4 */
uint64_t *base_pml4;

#ifdef FILESYS
/* -f: 파일 시스템을 포맷할 것인가? */
static bool format_filesys;
#endif

/* -q: 커널 작업 완료 후 전원을 끌 것인가? */
bool power_off_when_done;

bool thread_tests;

static void bss_init (void);
static void paging_init (uint64_t mem_end);

static char **read_command_line (void);
static char **parse_options (char **argv);
static void run_actions (char **argv);
static void usage (void);

static void print_stats (void);


int main (void) NO_RETURN;

/* Pintos 메인 프로그램 */
int
main (void) {
	uint64_t mem_end;
	char **argv;

	/* BSS 영역을 초기화하고 시스템의 RAM 크기를 얻는다 */
	bss_init ();

	/* 커맨드라인을 인자로 분할하고 옵션을 파싱한다 */
	argv = read_command_line ();
	argv = parse_options (argv);

	/* 락을 사용할 수 있도록 스레드로 초기화한 후
	   콘솔 락을 활성화한다 */
	thread_init ();
	console_init ();

	/* 메모리 시스템을 초기화한다 */
	mem_end = palloc_init ();
	malloc_init ();
	paging_init (mem_end);

#ifdef USERPROG
	tss_init ();
	gdt_init ();
#endif

	/* 인터럽트 핸들러를 초기화한다 */
	intr_init ();
	timer_init ();
	kbd_init ();
	input_init ();
#ifdef USERPROG
	exception_init ();
	syscall_init ();
#endif
	/* 스레드 스케줄러를 시작하고 인터럽트를 활성화한다 */
	thread_start ();
	serial_init_queue ();
	timer_calibrate ();

#ifdef FILESYS
	/* 파일 시스템을 초기화한다 */
	disk_init ();
	filesys_init (format_filesys);
#endif

#ifdef VM
	vm_init ();
#endif

	printf ("Boot complete.\n");

	/* 커널 커맨드라인에서 지정된 액션들을 실행한다 */
	run_actions (argv);

	/* 마무리 작업 */
	if (power_off_when_done)
		power_off ();
	thread_exit ();
}

/* BSS 영역 초기화 */
static void
bss_init (void) {
	/* "BSS"는 0으로 초기화되어야 하는 세그먼트이다.
	   실제로는 디스크에 저장되지 않고 커널 로더에 의해 0으로 초기화되지도 않으므로,
	   직접 0으로 초기화해야 한다.

	   BSS 세그먼트의 시작과 끝은 링커에 의해 _start_bss와 _end_bss로 기록된다.
	   kernel.lds를 참조하라. */
	extern char _start_bss, _end_bss;
	memset (&_start_bss, 0, &_end_bss - &_start_bss);
}

/* 페이지 테이블을 커널 가상 매핑으로 채우고,
 * CPU가 새로운 페이지 디렉토리를 사용하도록 설정한다.
 * base_pml4가 생성된 pml4를 가리키도록 한다. */
static void
paging_init (uint64_t mem_end) {
	uint64_t *pml4, *pte;
	int perm;
	pml4 = base_pml4 = palloc_get_page (PAL_ASSERT | PAL_ZERO);

	extern char start, _end_kernel_text;
	// 물리 주소 [0 ~ mem_end]를 
	//   [LOADER_KERN_BASE ~ LOADER_KERN_BASE + mem_end]로 매핑한다.
	for (uint64_t pa = 0; pa < mem_end; pa += PGSIZE) {
		uint64_t va = (uint64_t) ptov(pa);

		perm = PTE_P | PTE_W;
		if ((uint64_t) &start <= va && va < (uint64_t) &_end_kernel_text)
			perm &= ~PTE_W;

		if ((pte = pml4e_walk (pml4, va, 1)) != NULL)
			*pte = pa | perm;
	}

	// cr3 레지스터를 다시 로드한다
	pml4_activate(0);
}

/* 커널 커맨드라인을 단어별로 분할하고 argv와 같은 배열로 반환한다 */
static char **
read_command_line (void) {
	static char *argv[LOADER_ARGS_LEN / 2 + 1];
	char *p, *end;
	int argc;
	int i;

	argc = *(uint32_t *) ptov (LOADER_ARG_CNT);
	p = ptov (LOADER_ARGS);
	end = p + LOADER_ARGS_LEN;
	for (i = 0; i < argc; i++) {
		if (p >= end)
			PANIC ("command line arguments overflow");

		argv[i] = p;
		p += strnlen (p, end - p) + 1;
	}
	argv[argc] = NULL;

	/* 커널 커맨드라인을 출력한다 */
	printf ("Kernel command line:");
	for (i = 0; i < argc; i++)
		if (strchr (argv[i], ' ') == NULL)
			printf (" %s", argv[i]);
		else
			printf (" '%s'", argv[i]);
	printf ("\n");

	return argv;
}

/* ARGV[]의 옵션들을 파싱하고
   첫 번째 비옵션 인자를 반환한다 */
static char **
parse_options (char **argv) {
	for (; *argv != NULL && **argv == '-'; argv++) {
		char *save_ptr;
		char *name = strtok_r (*argv, "=", &save_ptr);
		char *value = strtok_r (NULL, "", &save_ptr);

		if (!strcmp (name, "-h"))
			usage ();
		else if (!strcmp (name, "-q"))
			power_off_when_done = true;
#ifdef FILESYS
		else if (!strcmp (name, "-f"))
			format_filesys = true;
#endif
		else if (!strcmp (name, "-rs"))
			random_init (atoi (value));
		else if (!strcmp (name, "-mlfqs"))
			thread_mlfqs = true;
#ifdef USERPROG
		else if (!strcmp (name, "-ul"))
			user_page_limit = atoi (value);
		else if (!strcmp (name, "-threads-tests"))
			thread_tests = true;
#endif
		else
			PANIC ("unknown option `%s' (use -h for help)", name);
	}

	return argv;
}

/* ARGV[1]에서 지정된 작업을 실행한다 */
static void
run_task (char **argv) {
	const char *task = argv[1];

	printf ("Executing '%s':\n", task);
#ifdef USERPROG
	if (thread_tests){
		run_test (task);
	} else {
		process_wait (process_create_initd (task));
	}
#else
	run_test (task);
#endif
	printf ("Execution of '%s' complete.\n", task);
}

/* ARGV[]에서 지정된 모든 액션들을 
   널 포인터 센티널까지 실행한다 */
static void
run_actions (char **argv) {
	/* 액션 구조체 */
	struct action {
		char *name;                       /* 액션 이름 */
		int argc;                         /* 액션 이름을 포함한 인자 개수 */
		void (*function) (char **argv);   /* 액션을 실행할 함수 */
	};

	/* 지원되는 액션들의 테이블 */
	static const struct action actions[] = {
		{"run", 2, run_task},
#ifdef FILESYS
		{"ls", 1, fsutil_ls},
		{"cat", 2, fsutil_cat},
		{"rm", 2, fsutil_rm},
		{"put", 2, fsutil_put},
		{"get", 2, fsutil_get},
#endif
		{NULL, 0, NULL},
	};

	while (*argv != NULL) {
		const struct action *a;
		int i;

		/* 액션 이름을 찾는다 */
		for (a = actions; ; a++)
			if (a->name == NULL)
				PANIC ("unknown action `%s' (use -h for help)", *argv);
			else if (!strcmp (*argv, a->name))
				break;

		/* 필수 인자들을 확인한다 */
		for (i = 1; i < a->argc; i++)
			if (argv[i] == NULL)
				PANIC ("action `%s' requires %d argument(s)", *argv, a->argc - 1);

		/* 액션을 실행하고 다음으로 진행한다 */
		a->function (argv);
		argv += a->argc;
	}

}

/* 커널 커맨드라인 도움말 메시지를 출력하고 시스템을 종료한다 */
static void
usage (void) {
	printf ("\nCommand line syntax: [OPTION...] [ACTION...]\n"
			"Options must precede actions.\n"
			"Actions are executed in the order specified.\n"
			"\nAvailable actions:\n"
#ifdef USERPROG
			"  run 'PROG [ARG...]' Run PROG and wait for it to complete.\n"
#else
			"  run TEST           Run TEST.\n"
#endif
#ifdef FILESYS
			"  ls                 List files in the root directory.\n"
			"  cat FILE           Print FILE to the console.\n"
			"  rm FILE            Delete FILE.\n"
			"Use these actions indirectly via `pintos' -g and -p options:\n"
			"  put FILE           Put FILE into file system from scratch disk.\n"
			"  get FILE           Get FILE from file system into scratch disk.\n"
#endif
			"\nOptions:\n"
			"  -h                 Print this help message and power off.\n"
			"  -q                 Power off VM after actions or on panic.\n"
			"  -f                 Format file system disk during startup.\n"
			"  -rs=SEED           Set random number seed to SEED.\n"
			"  -mlfqs             Use multi-level feedback queue scheduler.\n"
#ifdef USERPROG
			"  -ul=COUNT          Limit user memory to COUNT pages.\n"
#endif
			);
	power_off ();
}


/* 현재 실행 중인 머신의 전원을 끈다.
   Bochs나 QEMU에서 실행되는 경우에만 작동한다. */
void
power_off (void) {
#ifdef FILESYS
	filesys_done ();
#endif

	print_stats ();

	printf ("Powering off...\n");
	outw (0x604, 0x2000);               /* QEMU용 전원 끄기 명령 */
	for (;;);
}

/* Pintos 실행에 대한 통계를 출력한다 */
static void
print_stats (void) {
	timer_print_stats ();
	thread_print_stats ();
#ifdef FILESYS
	disk_print_stats ();
#endif
	console_print_stats ();
	kbd_print_stats ();
#ifdef USERPROG
	exception_print_stats ();
#endif
}
