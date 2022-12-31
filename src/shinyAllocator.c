/***
 * @filename shinyAllocator.c
 *
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
#include <stdint.h>
#include <stddef.h>

/***********************
 * Build configurations
 **********************/

#ifdef SHINYALLOCATOR_CONFIG_HEADER
#include SHINYALLOCATOR_CONFIG_HEADER
#endif

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

#if SHINYALLOCATOR_USE_INTRINSICS && !defined(SHINYALLOCATOR_LIKELY)
#if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#define SHINYALLOCATOR_LIKELY(x) __builtin_expect((x), 1)
#endif
#endif

#ifndef SHINYALLOCATOR_LIKELY
#define SHINYALLOCATOR_LIKELY(x) x
#endif

#ifndef SHINYALLOCATOR_PRIVATE
#define SHINYALLOCATOR_PRIVATE static inline
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

/***
 * @brief Mutex wrapper around FreeRTOS and Standard mutex interface
 */
#ifdef SHINYALLOCATOR_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Standard mutex type
typedef struct
{
    SemaphoreHandle_t handle;
} mutex_t;

// Initialize a mutex
SHINYALLOCATOR_PRIVATE int8_t mutex_init(mutex_t *mutex)
{
    mutex->handle = xSemaphoreCreateMutex();
    return (mutex->handle != NULL) ? 0 : -1;
}

// Destroy a mutex
SHINYALLOCATOR_PRIVATE int8_t mutex_destroy(mutex_t *mutex)
{
    vSemaphoreDelete(mutex->handle);
    return 0;
}

// Lock a mutex
SHINYALLOCATOR_PRIVATE int8_t mutex_lock(mutex_t *mutex)
{
    return xSemaphoreTake(mutex->handle, portMAX_DELAY) == pdTRUE ? 0 : -1;
}

// Try to lock a mutex
SHINYALLOCATOR_PRIVATE int8_t mutex_trylock(mutex_t *mutex)
{
    return xSemaphoreTake(mutex->handle, 0) == pdTRUE ? 0 : -1;
}
// Unlock a mutex
SHINYALLOCATOR_PRIVATE int8_t mutex_unlock(mutex_t *mutex)
{
    return xSemaphoreGive(mutex->handle) == pdTRUE ? 0 : -1;
}
#else
#include <pthread.h>

typedef pthread_mutex_t mutex_t;

SHINYALLOCATOR_PRIVATE int8_t mutex_init(mutex_t *mutex)
{
    return pthread_mutex_init(mutex, NULL);
}

SHINYALLOCATOR_PRIVATE int8_t mutex_destroy(mutex_t *mutex)
{
    return pthread_mutex_destroy(mutex);
}

SHINYALLOCATOR_PRIVATE int8_t mutex_lock(mutex_t *mutex)
{
    return pthread_mutex_lock(mutex);
}

SHINYALLOCATOR_PRIVATE int8_t mutex_trylock(mutex_t *mutex)
{
    return pthread_mutex_trylock(mutex);
}

SHINYALLOCATOR_PRIVATE int8_t mutex_unlock(mutex_t *mutex)
{
    return pthread_mutex_unlock(mutex);
}

#endif // SHINYALLOCATOR_FREERTOS

/****************************
 *  Encapsulated definitions
 ****************************/

// #if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
// #error "Unsupported language: ISO C99 or a newer version is required."
// #endif // __STDC_VERSION__ || (__STDC_VERSION__ < 199901L)

#if __STDC_VERSION__ < 201112L
#define static_assert(x, ...) typedef char _static_assert_gl(_static_assertion_, __LINE__)[(x) ? 1 : -1]
#define _static_assert_gl(a, b) _static_assert_gl_impl(a, b)
#define _static_assert_gl_impl(a, b) a##b
#endif // __STDC_VERSION__ <

/**
 * @brief The occupying space by the allocator is maximum SHINYALLOCATOR_ALIGNMENT bytes big and better to be bound
 * the over head with the maximum possible fragment size regarding the overhead of the allocation
 *
 */
#define FRAGMENT_SIZE_MIN (SHINYALLOCATOR_ALIGNMENT * 2U)
#define FRAGMENT_SIZE_MAX ((SIZE_MAX >> 1U) + 1U)

/**
 * @brief The maximum number of fragments that can be allocated to the pool
 */
#define NUM_FRAGMENTS_MAX (sizeof(size_t) * CHAR_BIT)

static_assert((SHINYALLOCATOR_ALIGNMENT & (SHINYALLOCATOR_ALIGNMENT - 1U)) == 0U, "SHINYALLOCATOR_ALIGNMENT not a power of 2");
static_assert((FRAGMENT_SIZE_MIN & (FRAGMENT_SIZE_MIN - 1U)) == 0U, "FRAGMENT_SIZE_MIN not a power of 2");
static_assert((FRAGMENT_SIZE_MAX & (FRAGMENT_SIZE_MAX - 1U)) == 0U, "FRAGMENT_SIZE_MAX not a power of 2");

typedef struct Fragment Fragment;
/**
 * @brief structuer to store fragment information
 *
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
 *
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
 *
 * @param fragments An array of pointers to all fragments
 * @param the binary Mask for representing the used/allocated fragments
 * @param diagnostics  The diagnostics associated with the pool
 */
struct shinyAllocatorInstance
{
    Fragment *fragments[NUM_FRAGMENTS_MAX];
    size_t nonEmptyFragmentMask;
    shinyAllocatorDiagnostics diagnostics;
};

/**
 * @brief the amount of space the aligned allocator instance takes
 */
#define INSTANCE_SIZE_PADDED ((sizeof(shinyAllocatorInstance) + SHINYALLOCATOR_ALIGNMENT - 1U) & ~(SHINYALLOCATOR_ALIGNMENT - 1U))
static_assert(INSTANCE_SIZE_PADDED >= sizeof(shinyAllocatorInstance), "Invalid instance footprint computation");
static_assert((INSTANCE_SIZE_PADDED % SHINYALLOCATOR_ALIGNMENT) == 0U, "Invalid instance footprint computation");

/**!
 * @brief efficient binary logarithm floor of x implementation
 *
 * @param x
 * @return floor of x log 2
 */
SHINYALLOCATOR_PRIVATE uint_fast8_t log2Floor(const size_t x)
{
    return (uint_fast8_t)(((sizeof(x) * CHAR_BIT) - 1U) - ((uint_fast8_t)SHINYALLOCATOR_CLZ(x)));
}

/**!
 * @brief efficient binary logarithm ceiling of x implementation
 *
 * @param x
 * @return floor of x log 2
 */
SHINYALLOCATOR_PRIVATE uint_fast8_t log2Ceil(const size_t x)
{
    return (x <= 1U) ? 0U : (uint_fast8_t)((sizeof(x) * CHAR_BIT) - ((uint_fast8_t)SHINYALLOCATOR_CLZ(x - 1U)));
}

/**!
 * @brief efficient x-th power of two implementation
 *
 * @param power
 * @return x-th power of 2
 */
SHINYALLOCATOR_PRIVATE size_t pow2(const uint_fast8_t power)
{
    return ((size_t)1U) << power;
}

/**
 * @param x
 * @return closest power of two
 */
SHINYALLOCATOR_PRIVATE size_t roundUpToPowerOfTwo(const size_t x)
{
    SHINYALLOCATOR_ASSERT(x >= 2U);
    return ((size_t)1U) << ((sizeof(x) * CHAR_BIT) - ((uint_fast8_t)SHINYALLOCATOR_CLZ(x - 1U)));
}

/**
 * @brief links the given fragments previous and next fragments in  a LeftToRight manner.
 *
 * @param left
 * @param right
 */
SHINYALLOCATOR_PRIVATE void fragmentLink(Fragment *left, Fragment *right)
{
    if (SHINYALLOCATOR_LIKELY(left != NULL))
    {
        left->header.next = right;
    }
    if (SHINYALLOCATOR_LIKELY(right != NULL))
    {
        right->header.prev = left;
    }
}

/***
 * @brief Appends a fragment to the allocator pool.
 *
 * @param handle pointer to the allocater handler
 * @param fragment pointer to the appending fragment
 */
SHINYALLOCATOR_PRIVATE void appendFragment(shinyAllocatorInstance *const handle, Fragment *const fragment)
{
    SHINYALLOCATOR_ASSERT(handle != NULL);
    SHINYALLOCATOR_ASSERT(fragment != NULL);
    SHINYALLOCATOR_ASSERT(fragment->header.size >= FRAGMENT_SIZE_MIN);
    SHINYALLOCATOR_ASSERT((fragment->header.size % FRAGMENT_SIZE_MIN) == 0U);
    const uint_fast8_t index = log2Floor(fragment->header.size / FRAGMENT_SIZE_MIN);
    SHINYALLOCATOR_ASSERT(index < NUM_FRAGMENTS_MAX);
    fragment->nextFree = handle->fragments[index];
    fragment->prevFree = NULL;
    if (SHINYALLOCATOR_LIKELY(handle->fragments[index] != NULL))
    {
        handle->fragments[index]->prevFree = fragment;
    }
    handle->fragments[index] = fragment;
    handle->nonEmptyFragmentMask |= pow2(index);
}

/***
 * @brief Removes a fragment from the allocator pool.
 *
 * @param handle pointer to the allocater handler
 * @param fragment pointer to the appending fragment
 */
SHINYALLOCATOR_PRIVATE void removeFragment(shinyAllocatorInstance *const handle, const Fragment *const fragment)
{
    SHINYALLOCATOR_ASSERT(handle != NULL);
    SHINYALLOCATOR_ASSERT(fragment != NULL);
    SHINYALLOCATOR_ASSERT(fragment->header.size >= FRAGMENT_SIZE_MIN);
    SHINYALLOCATOR_ASSERT((fragment->header.size % FRAGMENT_SIZE_MIN) == 0U);
    const uint_fast8_t index = log2Floor(fragment->header.size / FRAGMENT_SIZE_MIN);
    SHINYALLOCATOR_ASSERT(index < NUM_FRAGMENTS_MAX);

    if (SHINYALLOCATOR_LIKELY(fragment->nextFree != NULL))
    {
        fragment->nextFree->prevFree = fragment->prevFree;
    }
    if (SHINYALLOCATOR_LIKELY(fragment->prevFree != NULL))
    {
        fragment->prevFree->nextFree = fragment->nextFree;
    }

    if (SHINYALLOCATOR_LIKELY(handle->fragments[index] == fragment))
    {
        SHINYALLOCATOR_ASSERT(fragment->prevFree == NULL);
        handle->fragments[index] = fragment->nextFree;
        if (SHINYALLOCATOR_LIKELY(handle->fragments[index] == NULL))
        {
            handle->nonEmptyFragmentMask &= ~pow2(index);
        }
    }
}

/*********************************
 * Public interface implementation
 **********************************/

size_t sizeof_shinyAllocatorInstance(void)
{
    return sizeof(shinyAllocatorInstance);
};

shinyAllocatorDiagnostics shinyGetDiagnostics(shinyAllocatorInstance *handle)
{
    shinyAllocatorDiagnostics diagnostics = {
        .capacity = 0U,
        .allocated = 0U,
        .peakAllocated = 0U,
        .peakRequestSize = 0U,
        .outOfMemeoryCount = 0U};
    if (handle)
    {
        diagnostics = handle->diagnostics;
    }
    return diagnostics;
}

shinyAllocatorInstance *shinyInit(void *const base, const size_t size)
{
    shinyAllocatorInstance *out = NULL;
    if ((base != NULL) && ((((size_t)base) % SHINYALLOCATOR_ALIGNMENT) == 0U) &&
        (size >= (INSTANCE_SIZE_PADDED + FRAGMENT_SIZE_MIN)))
    {
        SHINYALLOCATOR_ASSERT(((size_t)base) % sizeof(shinyAllocatorInstance *) == 0U);
        out = (shinyAllocatorInstance *)base;
        out->nonEmptyFragmentMask = 0U;
        for (size_t i = 0; i < NUM_FRAGMENTS_MAX; i++)
        {
            out->fragments[i] = NULL;
        }

        size_t capacity = size - INSTANCE_SIZE_PADDED;
        if (capacity > FRAGMENT_SIZE_MAX)
        {
            capacity = FRAGMENT_SIZE_MAX;
        }
        while ((capacity % FRAGMENT_SIZE_MIN) != 0)
        {
            SHINYALLOCATOR_ASSERT(capacity > 0U);
            capacity--;
        }
        SHINYALLOCATOR_ASSERT((capacity % FRAGMENT_SIZE_MIN) == 0);
        SHINYALLOCATOR_ASSERT((capacity >= FRAGMENT_SIZE_MIN) && (capacity <= FRAGMENT_SIZE_MAX));

        Fragment *const frag = (Fragment *)(void *)(((char *)base) + INSTANCE_SIZE_PADDED);
        SHINYALLOCATOR_ASSERT((((size_t)frag) % SHINYALLOCATOR_ALIGNMENT) == 0U);
        frag->header.next = NULL;
        frag->header.prev = NULL;
        frag->header.size = capacity;
        frag->header.used = false;
        frag->nextFree = NULL;
        frag->prevFree = NULL;
        appendFragment(out, frag);
        SHINYALLOCATOR_ASSERT(out->nonEmptyFragmentMask != 0U);

        out->diagnostics.capacity = capacity;
        out->diagnostics.allocated = 0U;
        out->diagnostics.peakAllocated = 0U;
        out->diagnostics.peakRequestSize = 0U;
        out->diagnostics.outOfMemeoryCount = 0U;
    }

    return out;
}
void *shinyAllocate(shinyAllocatorInstance *const handle, const size_t amount)
{
    SHINYALLOCATOR_ASSERT(handle != NULL);
    SHINYALLOCATOR_ASSERT(handle->diagnostics.capacity <= FRAGMENT_SIZE_MAX);
    void *out = NULL;
    if (SHINYALLOCATOR_LIKELY((amount > 0U) && (amount <= (handle->diagnostics.capacity - SHINYALLOCATOR_ALIGNMENT))))
    {
        const size_t fragmentSize = roundUpToPowerOfTwo(amount + SHINYALLOCATOR_ALIGNMENT);
        SHINYALLOCATOR_ASSERT(fragmentSize <= FRAGMENT_SIZE_MAX);
        SHINYALLOCATOR_ASSERT(fragmentSize >= FRAGMENT_SIZE_MIN);
        SHINYALLOCATOR_ASSERT(fragmentSize >= amount + SHINYALLOCATOR_ALIGNMENT);
        SHINYALLOCATOR_ASSERT((fragmentSize & (fragmentSize - 1U)) == 0U);

        const uint_fast8_t optimalFragmentIndex = log2Ceil(fragmentSize / FRAGMENT_SIZE_MIN);
        SHINYALLOCATOR_ASSERT(optimalFragmentIndex < NUM_FRAGMENTS_MAX);
        const size_t candidateFragmentMask = ~(pow2(optimalFragmentIndex) - 1U);

        const size_t suitableFragments = handle->nonEmptyFragmentMask & candidateFragmentMask;
        const size_t smallestFragmentMask = suitableFragments & ~(suitableFragments - 1U);

        if (SHINYALLOCATOR_LIKELY(smallestFragmentMask != 0))
        {
            SHINYALLOCATOR_ASSERT((smallestFragmentMask & (smallestFragmentMask - 1U)) == 0U);
            const uint_fast8_t fragmentIndex = log2Floor(smallestFragmentMask);
            SHINYALLOCATOR_ASSERT(fragmentIndex >= optimalFragmentIndex);
            SHINYALLOCATOR_ASSERT(fragmentIndex < NUM_FRAGMENTS_MAX);

            Fragment *const frag = handle->fragments[fragmentIndex];
            SHINYALLOCATOR_ASSERT(frag != NULL);
            SHINYALLOCATOR_ASSERT(frag->header.size >= fragmentSize);
            SHINYALLOCATOR_ASSERT((frag->header.size % FRAGMENT_SIZE_MIN) == 0U);
            SHINYALLOCATOR_ASSERT(!frag->header.used);
            removeFragment(handle, frag);

            const size_t leftover = frag->header.size - fragmentSize;
            frag->header.size = fragmentSize;
            SHINYALLOCATOR_ASSERT(leftover < handle->diagnostics.capacity);
            SHINYALLOCATOR_ASSERT(leftover % FRAGMENT_SIZE_MIN == 0U);
            if (SHINYALLOCATOR_LIKELY(leftover >= FRAGMENT_SIZE_MIN))
            {
                Fragment *const newFrag = (Fragment *)(void *)(((char *)frag) + fragmentSize);
                SHINYALLOCATOR_ASSERT(((size_t)newFrag) % SHINYALLOCATOR_ALIGNMENT == 0U);
                newFrag->header.size = leftover;
                newFrag->header.used = false;
                fragmentLink(newFrag, frag->header.next);
                fragmentLink(frag, newFrag);
                appendFragment(handle, newFrag);
            }

            SHINYALLOCATOR_ASSERT((handle->diagnostics.allocated % FRAGMENT_SIZE_MIN) == 0U);
            handle->diagnostics.allocated += fragmentSize;
            SHINYALLOCATOR_ASSERT(handle->diagnostics.allocated <= handle->diagnostics.capacity);
            if (SHINYALLOCATOR_LIKELY(handle->diagnostics.peakAllocated < handle->diagnostics.allocated))
            {
                handle->diagnostics.peakAllocated = handle->diagnostics.allocated;
            }

            SHINYALLOCATOR_ASSERT(frag->header.size >= amount + SHINYALLOCATOR_ALIGNMENT);
            frag->header.used = true;

            out = ((char *)frag) + SHINYALLOCATOR_ALIGNMENT;
        }
    }

    if (SHINYALLOCATOR_LIKELY(handle->diagnostics.peakRequestSize < amount))
    {
        handle->diagnostics.peakRequestSize = amount;
    }
    if (SHINYALLOCATOR_LIKELY((out == NULL) && (amount > 0U)))
    {
        handle->diagnostics.outOfMemeoryCount++;
    }

    return out;
}

void shinyFree(shinyAllocatorInstance *const handle, void *const pointer)
{

    SHINYALLOCATOR_ASSERT(handle != NULL);
    SHINYALLOCATOR_ASSERT(handle->diagnostics.capacity <= FRAGMENT_SIZE_MAX);
    if (SHINYALLOCATOR_LIKELY(pointer != NULL))
    {
        Fragment *const frag = (Fragment *)(void *)(((char *)pointer) - SHINYALLOCATOR_ALIGNMENT);

        SHINYALLOCATOR_ASSERT(((size_t)frag) % sizeof(Fragment *) == 0U);
        SHINYALLOCATOR_ASSERT(((size_t)frag) >= (((size_t)handle) + INSTANCE_SIZE_PADDED));
        SHINYALLOCATOR_ASSERT(((size_t)frag) <=
                              (((size_t)handle) + INSTANCE_SIZE_PADDED + handle->diagnostics.capacity - FRAGMENT_SIZE_MIN));
        SHINYALLOCATOR_ASSERT(frag->header.used);
        SHINYALLOCATOR_ASSERT(((size_t)frag->header.next) % sizeof(Fragment *) == 0U);
        SHINYALLOCATOR_ASSERT(((size_t)frag->header.prev) % sizeof(Fragment *) == 0U);
        SHINYALLOCATOR_ASSERT(frag->header.size >= FRAGMENT_SIZE_MIN);
        SHINYALLOCATOR_ASSERT(frag->header.size <= handle->diagnostics.capacity);
        SHINYALLOCATOR_ASSERT((frag->header.size % FRAGMENT_SIZE_MIN) == 0U);

        frag->header.used = false;

        SHINYALLOCATOR_ASSERT(handle->diagnostics.allocated >= frag->header.size);
        handle->diagnostics.allocated -= frag->header.size;

        Fragment *const prev = frag->header.prev;
        Fragment *const next = frag->header.next;
        const bool join_left = (prev != NULL) && (!prev->header.used);
        const bool join_right = (next != NULL) && (!next->header.used);

        if (join_left && join_right)
        {
            removeFragment(handle, prev);
            removeFragment(handle, next);
            prev->header.size += frag->header.size + next->header.size;
            frag->header.size = 0;
            next->header.size = 0;
            SHINYALLOCATOR_ASSERT((prev->header.size % FRAGMENT_SIZE_MIN) == 0U);
            fragmentLink(prev, next->header.next);
            removeFragment(handle, prev);
        }
        else if (join_left)
        {
            removeFragment(handle, prev);
            prev->header.size += frag->header.size;
            frag->header.size = 0;
            SHINYALLOCATOR_ASSERT((prev->header.size % FRAGMENT_SIZE_MIN) == 0U);
            fragmentLink(prev, next);
            removeFragment(handle, prev);
        }
        else if (join_right)
        {
            removeFragment(handle, next);
            frag->header.size += next->header.size;
            next->header.size = 0;
            SHINYALLOCATOR_ASSERT((frag->header.size % FRAGMENT_SIZE_MIN) == 0U);
            fragmentLink(frag, next->header.next);
            removeFragment(handle, frag);
        }
        else
        {
            removeFragment(handle, frag);
        }
    }
}