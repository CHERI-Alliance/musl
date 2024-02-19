#include <string.h>
#include <stdint.h>
#include <endian.h>

/* This is the default memcpy code with extra cases to handle
   capability-aligned copies, so that everything that must be a capability
   uses a capability copy */

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

#define SIZEOF_CAP (sizeof(intcap_t))

	typedef intcap_t __attribute__((__may_alias__)) ucap;
	typedef uint64_t __attribute__((__may_alias__)) u64;
	typedef uint32_t __attribute__((__may_alias__)) u32;
	typedef uint16_t __attribute__((__may_alias__)) u16;

	for (; (ucap)s % SIZEOF_CAP && n; n--) *d++ = *s++;

    // s is now aligned to a capability, d is byte aligned

	if ((ucap)d % SIZEOF_CAP == 0) {
        // d is aligned to a capability
		for (; n>=(4 * SIZEOF_CAP); s+=(4 * SIZEOF_CAP), d+=(4 * SIZEOF_CAP), n-=(4 * SIZEOF_CAP)) {
			*(ucap *)(d+0) = *(ucap *)(s+0);
			*(ucap *)(d+SIZEOF_CAP) = *(ucap *)(s+SIZEOF_CAP);
			*(ucap *)(d+(2 * SIZEOF_CAP)) = *(ucap *)(s+(2 * SIZEOF_CAP));
			*(ucap *)(d+(3 * SIZEOF_CAP)) = *(ucap *)(s+(3 * SIZEOF_CAP));
		}
        // Now we know n < (4 * SIZEOF_CAP).
        // Do not bother to update n, just test the bits in it
        if (n & (2 * SIZEOF_CAP)) {   // 16 or 32
			*(ucap *)(d+0) = *(ucap *)(s+0);
			*(ucap *)(d+SIZEOF_CAP) = *(ucap *)(s+SIZEOF_CAP);
            d += (2 * SIZEOF_CAP); s += (2 * SIZEOF_CAP);
        }
        if (n & SIZEOF_CAP) {    // 8 or 16
			*(ucap *)(d+0) = *(ucap *)(s+0);
            d += SIZEOF_CAP; s += SIZEOF_CAP;
        }
		if (SIZEOF_CAP > 8 && (n&8)) {
			*(u64 *)(d+0) = *(u64 *)(s+0);
			d += 8; s += 8;
		}
		if (n&4) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			d += 4; s += 4;
		}
		if (n&2) {
            *(u16 *)(d+0) = *(u16 *)(s+0);
		}
		if (n&1) {
			*d = *s;
		}
		return dest;
	}

    // If we reach here, we know the incoming src and dest had different alignments,
    // so no need to worry about capabilities.
    // TODO: Use 64-bit load/store on 64-bit target
	uint32_t w, x;
	if (n >= 32) switch ((uintptr_t)d % 4) {
	case 1:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		n -= 3;
		for (; n>=17; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+1);
			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+5);
			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
			x = *(u32 *)(s+9);
			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+13);
			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
		}
		break;
	case 2:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		n -= 2;
		for (; n>=18; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+2);
			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+6);
			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
			x = *(u32 *)(s+10);
			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+14);
			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
		}
		break;
	case 3:
		w = *(u32 *)s;
		*d++ = *s++;
		n -= 1;
		for (; n>=19; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+3);
			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+7);
			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
			x = *(u32 *)(s+11);
			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+15);
			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
		}
		break;
	}
	if (n&16) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&8) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&4) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&2) {
		*d++ = *s++; *d++ = *s++;
	}
	if (n&1) {
		*d = *s;
	}
	return dest;
}
