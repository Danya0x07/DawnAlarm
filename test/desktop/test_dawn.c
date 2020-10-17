#include <dawn.h>
#include <unity.h>

static void assert_hour_span(int starthour, int endhour, bool expected)
{
    for (int hour = starthour; hour <= endhour; hour++) {
        for (int minute = 0; minute < 60; minute++)
            TEST_ASSERT_EQUAL(expected, dawn_is_ongoing(hour * 100 + minute));
    }
}

void test_dawn_ongoing_normal(void)
{
    dawn_setup(830, 30);
    assert_hour_span(0, 7, false);
    TEST_ASSERT_FALSE(dawn_is_ongoing(800));
    TEST_ASSERT_TRUE(dawn_is_ongoing(801));
    TEST_ASSERT_TRUE(dawn_is_ongoing(830));
    TEST_ASSERT_FALSE(dawn_is_ongoing(831));
    assert_hour_span(9, 23, false);
}

void test_dawn_ongoing_crazy_1(void)
{
    dawn_setup(30, 60);
    assert_hour_span(1, 22, false);
    TEST_ASSERT_FALSE(dawn_is_ongoing(2330));
    TEST_ASSERT_TRUE(dawn_is_ongoing(2331));
    TEST_ASSERT_TRUE(dawn_is_ongoing(0));
    TEST_ASSERT_TRUE(dawn_is_ongoing(30));
    TEST_ASSERT_FALSE(dawn_is_ongoing(31))
}

void test_dawn_ongoing_crazy_2(void)
{
    dawn_setup(0, 20);
    assert_hour_span(1, 22, false);
    TEST_ASSERT_FALSE(dawn_is_ongoing(2340));
    TEST_ASSERT_TRUE(dawn_is_ongoing(2341));
    TEST_ASSERT_TRUE(dawn_is_ongoing(0));
    TEST_ASSERT_FALSE(dawn_is_ongoing(1));
}