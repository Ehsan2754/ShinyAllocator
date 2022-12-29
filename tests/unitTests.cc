

#include <gtest/gtest.h>

 uint_fast8_t SHINYALLOCATOR_CLZ(const size_t x)
{

    size_t t = ((size_t)1U) << ((sizeof(size_t) * 8) - 1U);
    uint_fast8_t r = 0;
    while ((x & t) == 0)
    {
        t >>= 1U;
        r++;
    }
    return r;
}
uint_fast8_t log2Floor(const size_t x)
{
    return (uint_fast8_t)(((sizeof(x) * 8) - 1U) - ((uint_fast8_t)SHINYALLOCATOR_CLZ(x)));
}
namespace
{
    TEST(log2FloorTEST, HARDCODE)
    {
        EXPECT_EQ(log2Floor(1), 0);
        EXPECT_EQ(log2Floor(2), 1);
        EXPECT_EQ(log2Floor(3), 1);
        EXPECT_EQ(log2Floor(4), 2);
        EXPECT_EQ(log2Floor(30), 4);
        EXPECT_EQ(log2Floor(60), 5);
        EXPECT_EQ(log2Floor(64), 6);
    }
}