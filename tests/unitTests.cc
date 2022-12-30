#include <gtest/gtest.h>
#include "shinyAllocator.h"

#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>

using namespace std;
namespace
{

    const size_t KiB = 1024U;
    const size_t MiB = KiB * KiB;

    /**
     * @brief shinyInit() API test
     */
    TEST(shinyInitTest, emptyPoolCapacityTest)
    {
        const size_t arenaSize = 1E4;
        void *arena = (char *)aligned_alloc(64, arenaSize);
        auto pool = shinyInit(NULL, 0U);
        EXPECT_EQ(pool, (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).capacity, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
        pool = shinyInit(arena, 0U);
        EXPECT_EQ(pool, (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).capacity, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
        pool = shinyInit(arena, 1e2);
        EXPECT_EQ(shinyGetDiagnostics(pool).capacity, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
        pool = shinyInit(arena, 1e3);
        EXPECT_NE(pool, (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).capacity, 384U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
    }

    /**
     * @brief shinyAllocate() API test
     */
    TEST(shinyAllocateTest, allocationBoundaryVerification)
    {
        const size_t MiB256 = MiB * 256;
        const size_t arenaSize = MiB + MiB256;
        void *arena = (char *)aligned_alloc(64, arenaSize);

        auto pool = shinyInit(arena, arenaSize);

        EXPECT_NE(pool, (shinyAllocatorInstance *)NULL);

        EXPECT_GT(shinyGetDiagnostics(pool).capacity, arenaSize - 1024U);
        EXPECT_LT(shinyGetDiagnostics(pool).capacity, arenaSize);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0);

        EXPECT_EQ(shinyAllocate(pool, arenaSize), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 1U);

        EXPECT_EQ(shinyAllocate(pool, arenaSize - SHINYALLOCATOR_ALIGNMENT), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 2U);

        EXPECT_EQ(shinyAllocate(pool, shinyGetDiagnostics(pool).capacity - SHINYALLOCATOR_ALIGNMENT + 1U), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 3U);

        EXPECT_EQ(shinyAllocate(pool, arenaSize * 1e4), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4U);

        EXPECT_EQ(shinyAllocate(pool, 0U), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, arenaSize * 1e4);

        EXPECT_NE(shinyAllocate(pool, MiB256 - SHINYALLOCATOR_ALIGNMENT), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, MiB256);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, MiB256);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, arenaSize * 1e4);

        free(arena);
    }
    /**
     * @brief shinyFree() API test
     */
    TEST(shinyFreeTest, deAllocationLeftRightVerification)
    {
        const size_t KiB4 = KiB * 4;
        const size_t arenaSize = KiB4 + sizeof_shinyAllocatorInstance() + SHINYALLOCATOR_ALIGNMENT + 1U;
        void *arena = (char *)aligned_alloc(128, arenaSize);

        auto pool = shinyInit(arena, arenaSize);
        EXPECT_NE(pool, (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyAllocate(pool, 0U), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        auto ptr = shinyAllocate(pool, KiB4-SHINYALLOCATOR_ALIGNMENT);
        EXPECT_NE(ptr, (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount,0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, KiB4);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, KiB4-SHINYALLOCATOR_ALIGNMENT);
        
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, KiB4);
        shinyFree(pool, ptr);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated,0);

    }
}