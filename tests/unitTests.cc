#include <gtest/gtest.h>
#include "shinyAllocator.h"
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
        char arena[arenaSize] __attribute__((aligned(64)));
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
    TEST(shinyAllocateTest, emptyPoolCapacityTest)
    {
        const size_t MiB256 = MiB * 256;
        const size_t arenaSize = MiB + MiB256;
        char arena[arenaSize] __attribute__((aligned(64U)));
        // auto pool = shinyInit(arena, arenaSize);
        // EXPECT_NE(pool,(shinyAllocatorInstance*)NULL);

    }

}