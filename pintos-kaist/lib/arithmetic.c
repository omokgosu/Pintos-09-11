#include <stdint.h>

/* x86에서 64비트 정수를 다른 64비트 정수로 나누는 것은 
   단일 명령어나 짧은 시퀀스로 수행할 수 없습니다. 따라서 GCC는
   64비트 나눗셈과 나머지 연산을 함수 호출을 통해 구현합니다.
   이러한 함수들은 보통 libgcc에서 얻어지며, 이는 GCC가 수행하는
   모든 링크에 자동으로 포함됩니다.

   하지만 일부 x86-64 머신은 필요한 라이브러리(libgcc 포함) 없이
   32비트 x86 코드를 생성할 수 있는 컴파일러와 유틸리티를 가지고 있습니다.
   따라서 Pintos가 필요로 하는 libgcc의 유일한 루틴인 64비트 나눗셈 루틴을
   직접 구현함으로써 이러한 머신에서도 Pintos가 작동하도록 할 수 있습니다.

   완전성도 이러한 루틴을 포함하는 또 다른 이유입니다. Pintos가 완전히
   자체 포함적이라면 그만큼 덜 신비로워집니다. */

/* x86 DIVL 명령어를 사용하여 64비트 N을 32비트 D로 나누어
   32비트 몫을 구합니다. 몫을 반환합니다.
   몫이 32비트에 맞지 않으면 나눗셈 오류(#DE)로 트랩됩니다. */
static inline uint32_t
divl (uint64_t n, uint32_t d) {
	uint32_t n1 = n >> 32;
	uint32_t n0 = n;
	uint32_t q, r;

	asm ("divl %4"
			: "=d" (r), "=a" (q)
			: "0" (n1), "1" (n0), "rm" (d));

	return q;
}

/* X에서 앞쪽 0 비트의 개수를 반환합니다.
   X는 0이 아니어야 합니다. */
static int
nlz (uint32_t x) {
	/* 이 기법은 이식 가능하지만, 특정 시스템에서는 더 나은 방법이 있습니다.
	   충분히 새로운 GCC를 사용하면 __builtin_clz()를 사용하여 GCC가 알고 있는
	   최적화된 방법을 활용할 수 있습니다. 또는 x86 BSR 명령어를 직접 사용할 수도 있습니다. */
	int n = 0;
	if (x <= 0x0000FFFF) {
		n += 16;
		x <<= 16;
	}
	if (x <= 0x00FFFFFF) {
		n += 8;
		x <<= 8;
	}
	if (x <= 0x0FFFFFFF) {
		n += 4;
		x <<= 4;
	}
	if (x <= 0x3FFFFFFF) {
		n += 2;
		x <<= 2;
	}
	if (x <= 0x7FFFFFFF)
		n++;
	return n;
}

/* 부호 없는 64비트 N을 부호 없는 64비트 D로 나누어 몫을 반환합니다. */
static uint64_t
udiv64 (uint64_t n, uint64_t d) {
	if ((d >> 32) == 0) {
		/* 정확성의 증명:

		   이 함수에서 정의된 대로 n, d, b, n1, n0라고 하자.
		   [x]를 x의 "바닥"이라고 하자. T = b[n1/d]라고 하자. d는 0이 아니라고 가정하자.
		   그러면:
		   [n/d] = [n/d] - T + T
		   = [n/d - T] + T                         아래 (1)에 의해
		   = [(b*n1 + n0)/d - T] + T               n의 정의에 의해
		   = [(b*n1 + n0)/d - dT/d] + T
		   = [(b(n1 - d[n1/d]) + n0)/d] + T
		   = [(b[n1 % d] + n0)/d] + T,             %의 정의에 의해
		   이것이 아래에서 계산되는 식입니다.

		   (1) 임의의 실수 x와 정수 i에 대해: [x] + i = [x + i]임을 주목하세요.

		   divl()이 트랩되지 않도록 하려면 [(b[n1 % d] + n0)/d]가
		   b보다 작아야 합니다. [n1 % d]와 n0가 각각 최대값인 d - 1과 b - 1을 가진다고 가정하면:
		   [(b(d - 1) + (b - 1))/d] < b
		   <=> [(bd - 1)/d] < b
		   <=> [b - 1/d] < b
		   이는 항진명제입니다.

		   따라서 이 코드는 정확하며 트랩되지 않습니다. */
		uint64_t b = 1ULL << 32;
		uint32_t n1 = n >> 32;
		uint32_t n0 = n;
		uint32_t d0 = d;

		return divl (b * (n1 % d0) + n0, d0) + b * (n1 / d0);
	} else {
		/* http://www.hackersdelight.org/revisions.pdf에서 제공하는
		   알고리즘과 증명을 기반으로 합니다. */
		if (n < d)
			return 0;
		else {
			uint32_t d1 = d >> 32;
			int s = nlz (d1);
			uint64_t q = divl (n >> 1, (d << s) >> 32) >> (31 - s);
			return n - (q - 1) * d < d ? q - 1 : q;
		}
	}
}

/* 부호 없는 64비트 N을 부호 없는 64비트 D로 나누어 나머지를 반환합니다. */
static uint32_t
umod64 (uint64_t n, uint64_t d) {
	return n - d * udiv64 (n, d);
}

/* 부호 있는 64비트 N을 부호 있는 64비트 D로 나누어 몫을 반환합니다. */
static int64_t
sdiv64 (int64_t n, int64_t d) {
	uint64_t n_abs = n >= 0 ? (uint64_t) n : -(uint64_t) n;
	uint64_t d_abs = d >= 0 ? (uint64_t) d : -(uint64_t) d;
	uint64_t q_abs = udiv64 (n_abs, d_abs);
	return (n < 0) == (d < 0) ? (int64_t) q_abs : -(int64_t) q_abs;
}

/* 부호 있는 64비트 N을 부호 있는 64비트 D로 나누어 나머지를 반환합니다. */
static int32_t
smod64 (int64_t n, int64_t d) {
	return n - d * sdiv64 (n, d);
}

/* 다음은 GCC가 호출하는 루틴들입니다. */

long long __divdi3 (long long n, long long d);
long long __moddi3 (long long n, long long d);
unsigned long long __udivdi3 (unsigned long long n, unsigned long long d);
unsigned long long __umoddi3 (unsigned long long n, unsigned long long d);

/* 부호 있는 64비트 나눗셈. */
long long
__divdi3 (long long n, long long d) {
	return sdiv64 (n, d);
}

/* 부호 있는 64비트 나머지. */
long long
__moddi3 (long long n, long long d) {
	return smod64 (n, d);
}

/* 부호 없는 64비트 나눗셈. */
unsigned long long
__udivdi3 (unsigned long long n, unsigned long long d) {
	return udiv64 (n, d);
}

/* 부호 없는 64비트 나머지. */
unsigned long long
__umoddi3 (unsigned long long n, unsigned long long d) {
	return umod64 (n, d);
}
