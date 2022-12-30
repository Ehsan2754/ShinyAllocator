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
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 1);

        EXPECT_EQ(shinyAllocate(pool, arenaSize - SHINYALLOCATOR_ALIGNMENT), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 2);

        EXPECT_EQ(shinyAllocate(pool, shinyGetDiagnostics(pool).capacity - SHINYALLOCATOR_ALIGNMENT + 1U), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 3);

        EXPECT_EQ(shinyAllocate(pool, arenaSize * 1e4), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4);

        EXPECT_EQ(shinyAllocate(pool, 0U), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, arenaSize * 1e4);

        EXPECT_NE(shinyAllocate(pool, MiB256 - SHINYALLOCATOR_ALIGNMENT), (shinyAllocatorInstance *)NULL);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 4);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, MiB256);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, MiB256);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, arenaSize * 1e4);

        free(arena);
    }

}