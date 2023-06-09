#include "co/unitest.h"
#include "co/mem.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <intrin.h>
#endif


namespace test {
namespace mem {

#ifdef _WIN32
#if __arch64
inline int _find_msb(size_t x) { /* x != 0 */
    unsigned long i;
    _BitScanReverse64(&i, x);
    return (int)i;
}

inline uint32 _find_lsb(size_t x) { /* x != 0 */
    unsigned long r;
    _BitScanForward64(&r, x);
    return r;
}

#else
inline int _find_msb(size_t x) { /* x != 0 */
    unsigned long i;
    _BitScanReverse(&i, x);
    return (int)i;
}

inline uint32 _find_lsb(size_t x) { /* x != 0 */
    unsigned long r;
    _BitScanForward(&r, x);
    return r;
}
#endif

inline uint32 _pow2_align(uint32 n) {
    unsigned long r;
    _BitScanReverse(&r, n - 1);
    return 2u << r;
}

#else
#if __arch64
inline int _find_msb(size_t x) { /* x != 0 */
    return 63 - __builtin_clzll(x);
}

inline uint32 _find_lsb(size_t x) { /* x != 0 */
    return __builtin_ffsll(x) - 1;
}

#else
inline int _find_msb(size_t v) { /* x != 0 */
    return 31 - __builtin_clz(v);
}

inline uint32 _find_lsb(size_t x) { /* x != 0 */
    return __builtin_ffs(x) - 1;
}
#endif

inline uint32 _pow2_align(uint32 n) {
    return 1u << (32 - __builtin_clz(n - 1));
}

#endif

} // mem

DEF_test(mem) {
    DEF_case(bitops) {
        EXPECT_EQ(mem::_find_lsb(1), 0);
        EXPECT_EQ(mem::_find_lsb(12), 2);
        EXPECT_EQ(mem::_find_lsb(3u << 20), 20);
      #if __arch64
        EXPECT_EQ(mem::_find_msb(1), 0);
        EXPECT_EQ(mem::_find_msb(12), 3);
        EXPECT_EQ(mem::_find_msb(3u << 20), 21);
        EXPECT_EQ(mem::_find_msb(1ull << 63), 63);
        EXPECT_EQ(mem::_find_msb(~0ull), 63);
        EXPECT_EQ(mem::_find_lsb(~0ull), 0);
      #else
        EXPECT_EQ(mem::_find_msb(1), 0);
        EXPECT_EQ(mem::_find_msb(12), 3);
        EXPECT_EQ(mem::_find_msb(3u << 20), 21);
        EXPECT_EQ(mem::_find_msb(1ull << 31), 31);
        EXPECT_EQ(mem::_find_msb(~0u), 31);
        EXPECT_EQ(mem::_find_lsb(~0u), 0);
      #endif
    }

    DEF_case(small) {
        void* p = co::alloc(2048);
        EXPECT_NE(p, (void*)0);
        *(uint32*)p = 7;
        EXPECT_EQ(*(uint32*)p, 7);
        co::free(p, 2048);

        void* x = p;
        p = co::alloc(8);
        EXPECT_EQ(p, x);
        co::free(p, 8);

        p = co::alloc(72);
        EXPECT_EQ(p, x);
        co::free(p, 72);

        p = co::alloc(4096);
        EXPECT_NE(p, (void*)0);
        EXPECT_NE(p, x);
        co::free(p, 4096);

        int* v = co::make<int>(7);
        EXPECT_EQ(*v, 7);
        co::del(v);
    }

    DEF_case(realloc) {
        void* p;
        p = co::alloc(48);
        EXPECT_NE(p, (void*)0);
        *(uint32*)p = 7;
        EXPECT_EQ(*(uint32*)p, 7);

        void* x = p;
        p = co::realloc(p, 48, 64);
        EXPECT_EQ(p, x);
        EXPECT_EQ(*(uint32*)p, 7);

        p = co::realloc(p, 64, 128);
        EXPECT_EQ(p, x);

        p = co::realloc(p, 128, 2048);
        EXPECT_EQ(p, x);

        p = co::realloc(p, 2048, 4096);
        EXPECT_NE(p, x);
        EXPECT_EQ(*(uint32*)p, 7);

        x = p;
        p = co::realloc(p, 4096, 8 * 1024);
        EXPECT_EQ(p, x);
        EXPECT_EQ(*(uint32*)p, 7);

        p = co::realloc(p, 8 *1024, 32 * 1024);
        EXPECT_EQ(p, x);

        p = co::realloc(p, 32 * 1024, 64 * 1024);
        EXPECT_EQ(p, x);

        x = p;
        p = co::realloc(p, 64 * 1024, 132 * 1024);
        EXPECT_NE(p, x);
        EXPECT_EQ(*(uint32*)p, 7);

        p = co::realloc(p, 132 * 1024, 256 * 1024);
        EXPECT_EQ(*(uint32*)p, 7);
        co::free(p, 256 * 1024);
    }

    DEF_case(static) {
        int* x = co::make_static<int>(7);
        EXPECT_NE(x, (void*)0);
        EXPECT_EQ(*x, 7);
    }

    static int gc = 0;
    static int gd = 0;

    struct A {
        A() {}
        virtual ~A() {}
    };

    struct B : A {
        B() { ++gc; }
        virtual ~B() { ++gd; }
    };

    DEF_case(unique) {
        co::unique<int> p;
        EXPECT(p == NULL);
        EXPECT(!p);

        p = co::make_unique<int>(7);
        EXPECT_EQ(*p, 7);
        *p = 3;
        EXPECT_EQ(*p, 3);

        auto q = co::make_unique<int>(7);
        EXPECT_EQ(*q, 7);

        q = std::move(p);
        EXPECT_EQ(*q, 3);
        EXPECT(p == NULL);

        p = q;
        EXPECT_EQ(*p, 3);
        EXPECT(q == NULL);

        p.swap(q);
        EXPECT_EQ(*q, 3);
        EXPECT(p == NULL);

        q.reset();
        EXPECT(q == NULL);

        co::unique<A> a = co::make_unique<B>();
        EXPECT_EQ(gc, 1);
        a.reset();
        EXPECT_EQ(gd, 1);
    }

    DEF_case(shared) {
        co::shared<int> p;
        EXPECT(p == NULL);
        EXPECT(!p);
        EXPECT_EQ(p.ref_count(), 0);

        co::shared<int> q(p);
        EXPECT_EQ(p.ref_count(), 0);
        EXPECT_EQ(q.ref_count(), 0);

        p = co::make_shared<int>(7);
        EXPECT_EQ(*p, 7);
        *p = 3;
        EXPECT_EQ(*p, 3);
        EXPECT_EQ(p.ref_count(), 1);
        EXPECT_EQ(q.ref_count(), 0);

        q = p;
        EXPECT_EQ(p.ref_count(), 2);
        EXPECT_EQ(q.ref_count(), 2);
        EXPECT_EQ(*q, 3);

        p.reset();
        EXPECT(p == NULL);
        EXPECT_EQ(q.ref_count(), 1);
        EXPECT_EQ(*q, 3);

        p.swap(q);
        EXPECT(q == NULL);
        EXPECT_EQ(p.ref_count(), 1);
        EXPECT_EQ(*p, 3);

        q = std::move(p);
        EXPECT(p == NULL);
        EXPECT_EQ(q.ref_count(), 1);
        EXPECT_EQ(*q, 3);

        co::shared<A> a = co::make_shared<B>();
        EXPECT_EQ(gc, 2);

        auto b = a;
        EXPECT_EQ(gc, 2);
        EXPECT_EQ(a.ref_count(), 2);

        b.reset();
        EXPECT_EQ(gd, 1);
        EXPECT_EQ(a.ref_count(), 1);

        a.reset();
        EXPECT_EQ(a.ref_count(), 0);
        EXPECT_EQ(gd, 2);
    }
}

} // namespace test
