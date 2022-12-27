/***
 * @filename shinyAllocator.c
 * @author Ehsan Shaghaei <ehsan2754@gmail.com>
 * @date Dec 27, 2022
 * @version 1.0.0
 * @brief source file for shinyAllocator library
 *
 * @copyright 2022 GNU GENERAL PUBLIC LICENSE
 *
 */
#include "shinyAllocator.h"
#include <assert.h>
#include <limits.h>

/**
 * Build configurations
 */

#ifndef SHINYALLOCATOR_ASSERT
#define SHINYALLOCATOR_ASSERT(x) assert(x)
#endif

/**
 * @brief Activates compiler optimizations for annotations
 *
 */
#ifndef SHINYALLOCATOR_USE_INTRINSICS
#define SHINYALLOCATOR_USE_INTRINSICS 1
#endif

#ifndef SHINYALLOCATOR_PRIVATE
#define SHINYALLOCATOR_PRIVATE static inline
#endif

#if SHINYALLOCATOR_USE_INTRINSICS && !defined(SHINYALLOCATOR_LIKELY)
#   if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#       define SHINYALLOCATOR_LIKELY(x) __builtin_expect((x), 1)
#   endif 
#endif

#ifndef SHINYALLOCATOR_LIKELY
#define SHINYALLOCATOR_LIKELY(x) x
#endif

#ifndef SHINYALLOCATOR_CLZ
SHINYALLOCATOR_PRIVATE uint_fast8_t SHINYALLOCATOR_CLZ(const size_t x)
{
    SHINYALLOCATOR_ASSERT(x > 0);
    size_t t = ((size_t)1U) << ((sizeof(size_t) * CHAR_BIT) - 1U);
    uint_fast8_t r = 0;
    while ((x & t) == 0)
    {
        t >>= 1U;
        r++;
    }
    return r;
}
#endif // SHINYALLOCATOR_CLZ

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
#error "Unsupported language: ISO C99 or a newer version is required."
#endif // __STDC_VERSION__ || (__STDC_VERSION__ < 199901L)

#if __STDC_VERSION__ < 201112L
#define static_assert(x, ...) typedef char _static_assert_gl(_static_assertion_, __LINE__)[(x) ? 1 : -1]
#define _static_assert_gl(a, b) _static_assert_gl_impl(a, b)
#define _static_assert_gl_impl(a, b) a##b
#endif // __STDC_VERSION__ <

/**
 * @brief The occupying space by the allocator is maximum SHINYALLOCATOR_ALIGNMENT bytes big
 *
 */
#define FRAGMENT_SIZE_MIN (SHINYALLOCATOR_ALIGNMENT * 2U)

/**
 * @brief if the allocation amount plus per-fragment overhead exceeds 2^(b-1),
 * where b is the pointer bit width, then ceil(log2(amount)) yields b; then 2^b causes an integer overflow.
 * To avoid this, we put a hard limit on fragment size (which is amount + per-fragment overhead): 2^(b-1)
 */
#define FRAGMENT_SIZE_MAX ((SIZE_MAX >> 1U) + 1U)

/**
 * @brief The maximum number of fragments that can be allocated to the pool
 */
#define NUM_FRAGMENTS_MAX (sizeof(size_t) * CHAR_BIT)

static_assert((SHINYALLOCATOR_ALIGNMENT & (SHINYALLOCATOR_ALIGNMENT - 1U)) == 0U, "SHINYALLOCATOR_ALIGNMENT not a power of 2");
static_assert((FRAGMENT_SIZE_MIN & (FRAGMENT_SIZE_MIN - 1U)) == 0U, "FRAGMENT_SIZE_MIN not a power of 2");
static_assert((FRAGMENT_SIZE_MAX & (FRAGMENT_SIZE_MAX - 1U)) == 0U, "FRAGMENT_SIZE_MAX not a power of 2");

typedef Fragment Fragment;

/**
 * @brief structuer to store fragment information
 * @param next stores the pointer to the next fragment
 * @param prev stores the pointer to the previous fragment
 * @param size stores the size of the fragment
 * @param used stores current used capacity of the fragment
 */
typedef struct FragmentHeader
{
    Fragment *next;
    Fragment *prev;
    size_t size;
    bool used;
} FragmentHeader;
static_assert(sizeof(FragmentHeader) <= SHINYALLOCATOR_ALIGNMENT, "Memory layout error");

/**
 * @brief Stores current fragment status
 * @param header The header fot the fragment
 * @param nextFree Pointer to the next free fragment in the pool (NULL for the last fragment of the pool)
 * @param prevFree Pointer to the previous free fragment in the pool (NULL for the first fragment of the pool)
 * */
struct Fragment
{
    FragmentHeader header;
    Fragment *nextFree;
    Fragment *prevFree;
};
static_assert(sizeof(Fragment) <= FRAGMENT_SIZE_MIN, "Memory layout error");

/**
 * @brief the allocator which stores the information about the pool structure
 * @param fragments An array of pointers to all fragments
 * @param the binary Mask for representing the used/allocated fragments
 * @param diagnostics  The diagnostics associated with the pool
 */
struct shinyAllocatorInstance
{
    Fragment *fragments[NUM_FRAGMENTS_MAX];
    size_t nonEmptyBinMask;
    shinyAllocatorDiagnostics diagnostics;
};

/**
 * @brief the amount of space the aligned allocator instance takes
 */
#define INSTANCE_SIZE_PADDED ((sizeof(shinyAllocatorInstance) + SHINYALLOCATOR_ALIGNMENT - 1U) & ~(SHINYALLOCATOR_ALIGNMENT - 1U))

static_assert(INSTANCE_SIZE_PADDED >= sizeof(shinyAllocatorInstance), "Invalid instance footprint computation");
static_assert((INSTANCE_SIZE_PADDED % SHINYALLOCATOR_ALIGNMENT) == 0U, "Invalid instance footprint computation");


/**!
 * @brief efficient binary logarithm of x implementation
 * 
 * @param x 
 * @return floor of x log 2 
 */
SHINYALLOCATOR_PRIVATE uint_fast8_t log2Floor(const size_t x)
{
    return (uint_fast8_t)(((sizeof(x) * CHAR_BIT) - 1U) - ((uint_fast8_t)SHINYALLOCATOR_CLZ(x)));
}
/**!
 * @brief efficient x-th power of two implementation
 * 
 * @param power 
 * @return x-th power of 2 
 */
SHINYALLOCATOR_PRIVATE size_t pow2(const uint_fast8_t power)
{
    return ((size_t) 1U) << power;
}

shinyAllocatorInstance *shinyInit(void *const base, const size_t size)
{
    shinyAllocatorInstance *allocator = NULL;
    if ((base != NULL) && ((((size_t)base) % SHINYALLOCATOR_ALIGNMENT) == 0U) &&
        (size >= (INSTANCE_SIZE_PADDED + FRAGMENT_SIZE_MIN)))
    {
        // TODO: check the base pointer can fit the allocator metadata
        allocator = (shinyAllocatorInstance *)base;
        allocator->nonEmptyBinMask = 0U;

        size_t capacity = size - INSTANCE_SIZE_PADDED;
        capacity = capacity > FRAGMENT_SIZE_MAX ? FRAGMENT_SIZE_MAX : capacity;

        for (; !(capacity % FRAGMENT_SIZE_MAX); capacity--)
        {
        };
        // TODO : check the alignment
        Fragment *const f = (Fragment *)(void *)(((char *)base) + INSTANCE_SIZE_PADDED);
        f->header.next = NULL;
        f->header.prev = NULL;
        f->header.size = capacity;
        f->header.used = false;
        f->nextFree = f;
        f->prevFree = f;

        const uint_fast8_t index = log2Floor(f->header.size / FRAGMENT_SIZE_MIN);
        f->nextFree = allocator->fragments[index];
        f->prevFree = NULL;
        if (SHIN_LIKELY(f->fragments[index] != NULL))
        {
            f->fragments[index]->prev_free = f;
        }

        f->fragment[index] = f;
        f->nonempty_bin_mask |= pow2(index);

        allocator->diagnostics.capacity = capacity;
        allocator->diagnostics.allocated = 0U;
        allocator->diagnostics.peakAllocated = 0U;
        allocator->diagnostics.peakRequestSize = 0U;
        allocator->diagnostics.outOfMemeoryCount = 0U;
    }
    return allocator;
}
