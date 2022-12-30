#include <gtest/gtest.h>
#include "shinyAllocator.h"
// using namespace std;
namespace
{

    size_t KiB = 1024U;
    size_t MiB = KiB * KiB;

    char arena[10000] __attribute__((aligned(128)));

    TEST(shinyInitTest, emptyPool)
    {
        // alignas(128) std::array<std::byte, 10'000U> arena{};
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
        pool = shinyInit(arena, 99U);
        EXPECT_EQ(shinyGetDiagnostics(pool).capacity, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).allocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakAllocated, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).peakRequestSize, 0U);
        EXPECT_EQ(shinyGetDiagnostics(pool).outOfMemeoryCount, 0U);
        // pool = shinyInit(arena, 1000U);
        // cout << endl << shinyGetDiagnostics(pool).capacity << endl;
    }

}