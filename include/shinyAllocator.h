/***
 * @brief header file for shinyAllocator library
 * @filename shinyAllocator.h
 * @author Ehsan Shaghaei <ehsan2754@gmail.com>
 * @date Dec 27, 2022
 * @version 1.0.0
 *
 * @copyright 2022 GNU GENERAL PUBLIC LICENSE
 *
 */
#ifndef __shinyAllocator_h
#define __shinyAllocator_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief static version number
 */
#define SHINYALLOCATOR_VERSION_MAJOR 1
#define SHINYALLOCATOR_VERSION_MINOR 0

/**
 * @brief Memory alignment based on platform pointer (8/16/32)
 *
 */
#define SHINYALLOCATOR_ALIGNMENT (sizeof(void *) * 4U)

    /**
     * @brief encapsulation of the structure instance
     */
    typedef struct shinyAllocatorInstance shinyAllocatorInstance;
    typedef struct shinyAllocatorThreadSafe shinyAllocatorThreadSafe;

    /**
     * @brief struct for performing runtime diagnostic tests
     *
     * @param capacity constant parameter which indicates the total size of the momory pool
     * @param allocated currently allocated memory (this values is alligned)
     * @param peakAllocated the maximum memory ever allocated in the pool(Max. allocated)
     * @param peakRequestSize the maximum size ever allocator tried to allocate
     * @param outOfMemeoryCount non-decreasing number of times the allocation request failed due to lack of memory
     */
    typedef struct
    {
        size_t capacity;
        size_t allocated;
        size_t peakAllocated;
        size_t peakRequestSize;
        size_t outOfMemeoryCount;
    } shinyAllocatorDiagnostics;

    /**
     * @return size of the shinyAllocatorInstance
     */
    size_t sizeof_shinyAllocatorInstance(void);
    /***
     * @param handle pointer
     * @return current diagnostics
     */
    shinyAllocatorDiagnostics shinyGetDiagnostics(shinyAllocatorInstance *handle);
    /**
     * @brief Initializes the shinyAllocator for the given base pointer and size.
     * @param base base pointer for the pool, it should be aligned to SHINYALLOCATOR_ALIGNMENT.
     * @param size size of the pool, this parameter should not exceed SIZE_MAX/2.
     * @details allocator occupy 40+ bytes (up to 600 bytes depending on architecture) of the pool for holding its configuration.
     * @returns NULL if the pool is not sufficient for the given size otherwise returns a pointer to the newly initialized allocator.
     * @note An initialized any resources, hence you can discard it without any de-initialization if it is not needed.
     */
    shinyAllocatorInstance *shinyInit(void *const base, const size_t size);

    /**
     * @brief Allocated the requested memory to the given the pool handle, returns NULL if it fails.
     * @param handle allocater handle to the pool.
     * @param size the requested allocation size.
     * @note may cause integer-overflow for bigger sizes. It's recommended keep overall allocated bytes less than SIZE_MAX/2-1.
     */
    void *shinyAllocate(shinyAllocatorInstance *const handle, const size_t amount);

    /**
     * @brief Frees the memory allocated to the given the pool handle.
     * @param handle allocator handle to the pool.
     * @param pointer pointer to the allocated memory.
     */
    void shinyFree(shinyAllocatorInstance *const handle, void *const pointer);

    /**
     * @brief Thread-safe wrapper for shinyGetDiagnostics().
     * @param threadSafeHandle Thread-safe shinyAllocator instance.
     * @return Diagnostics for the shinyAllocator instance.
     */
    shinyAllocatorDiagnostics shinyGetDiagnosticsThreadSafe(shinyAllocatorThreadSafe *const threadSafeHandle);

    /**
     * @brief Initializes a thread-safe shinyAllocator instance.
     * @param base Base address for the shinyAllocator instance.
     * @param size Size of the shinyAllocator instance.
     * @return Thread-safe shinyAllocator instance.
     */
    shinyAllocatorThreadSafe *shinyInitThreadSafe(void *const base, const size_t size);

    /**
     * @brief Allocates memory from a thread-safe shinyAllocator instance.
     * @param threadSafeHandle Thread-safe shinyAllocator instance.
     * @param amount Amount of memory to allocate.
     * @return Pointer to the allocated memory.
     */
    void *shinyAllocateThreadSafe(shinyAllocatorThreadSafe *const threadSafeHandle, const size_t amount);

    /**
     * @brief Frees memory allocated by a thread-safe shinyAllocator instance.
     * @param threadSafeHandle Thread-safe shinyAllocator instance.
     * @param pointer Pointer to the memory to be freed.
     */
    void shinyFreeThreadSafe(shinyAllocatorThreadSafe *const threadSafeHandle, void *const pointer);
#ifdef __cplusplus
}
#endif
#endif // __shinyAllocator_h