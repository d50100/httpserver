#include "utils.h"

void itimeofday(long* sec, long* usec)
{

    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;

}

int64_t iclock64(void)
{
    long s, u;
    int64_t value;
    itimeofday(&s, &u);
    value = ((int64_t)s) * 1000 + (u / 1000);
    return value;
}

uint32_t iclock()
{
    return (uint32_t)(iclock64() & 0xfffffffful);
}

uint32_t generate_conv() {
    srand(time(NULL));  // 使用当前时间作为随机数种子

    // 生成一个随机的32位无符号整数作为conv值
    uint32_t conv = rand();

    return conv;
}