#include <unity.h>

extern void test_dawn_ongoing_normal(void);
extern void test_dawn_ongoing_crazy_1(void);
extern void test_dawn_ongoing_crazy_2(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_dawn_ongoing_normal);
    RUN_TEST(test_dawn_ongoing_crazy_1);
    RUN_TEST(test_dawn_ongoing_crazy_2);

    return UNITY_END();
}