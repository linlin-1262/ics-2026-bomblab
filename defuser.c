#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);

    uint32_t a = low + UINT32_C(0x50ef);
    a ^= high;

    uint32_t b = low * UINT32_C(5);
    uint32_t shifted = middle + UINT32_C(0xa9a9);
    b ^= shifted;

    uint32_t result = b + UINT32_C(2) * a;
    result ^= UINT32_C(0x7e6b);

    values[0] = result;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = (uint32_t)((challenge >> 9) & UINT64_C(63))
                ^ UINT32_C(19);

    for (unsigned i = 1; i < 6; ++i) {
        unsigned shift = 6 + 2 * i;
        uint32_t part =
            (uint32_t)((challenge >> shift) & UINT64_C(15));

        uint32_t next = values[i - 1] * UINT32_C(4);
        next ^= part;
        next += (uint32_t)(shift - 6);
        values[i] = next;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)(challenge & UINT64_C(7));
    uint32_t x = (uint32_t)(challenge >> 32)
                 ^ (uint32_t)(challenge >> 3);
    uint32_t result;

    switch (index) {
        case 0:
            result = x * UINT32_C(5) + UINT32_C(0x685b);
            break;
        case 1:
            result = (x ^ UINT32_C(0xbd7c)) + (x >> 1);
            break;
        case 2:
            result = x * UINT32_C(5) + UINT32_C(0x15d1);
            break;
        case 3:
            result = (x ^ UINT32_C(0xb78c)) + (x >> 2);
            break;
        case 4:
            result = x * UINT32_C(5) + UINT32_C(0x25dcd);
            break;
        case 5:
            result = x * UINT32_C(5) + UINT32_C(0x4f5f);
            break;
        case 6:
            result = (x * UINT32_C(4)) ^ UINT32_C(0xfec4);
            break;
        case 7:
            result = (x * UINT32_C(4)) ^ UINT32_C(0x2ec1);
            break;
        default:
            return 1;
    }

    values[0] = index;
    values[1] = result;
    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + UINT32_C(0x98)) ^ key;
    }

    uint32_t child;
    uint32_t tag;

    if (target < mid) {
        child = phase4_walk(target, low, mid - 1, key);
        tag = UINT32_C(0x14);
    } else {
        child = phase4_walk(target, mid + 1, high, key);
        tag = UINT32_C(0x19);
    }

    uint32_t result = child * UINT32_C(3);
    uint32_t mix = mid + key;
    return result ^ mix ^ tag;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target =
        (uint32_t)(challenge & UINT64_C(127));
    uint32_t key =
        (uint32_t)(challenge >> 7)
        ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(target, 0, 127, key);
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    static const int next[12] = {
        1, 8, 9, -1, 7, 3, 5, 10, 6, 11, 2, 0
    };

    static const uint32_t field8[12] = {
        0x044e, 0x0731, 0x013d, 0x0a11,
        0x0572, 0x005f, 0x037f, 0x0b86,
        0x0816, 0x0282, 0x0680, 0x0968
    };

    static const uint32_t field12[12] = {
        0xf379, 0x8d5e, 0x9fc1, 0x6158,
        0x2e5c, 0x4db4, 0x4911, 0xcf02,
        0x1295, 0x6643, 0x04c1, 0x794c
    };

    static const int heads[8] = {
        4, 7, 10, 2, 9, 11, 0, 1
    };

    uint32_t index = (uint32_t)(challenge & UINT64_C(7));
    unsigned steps =
        (unsigned)((challenge >> 3) & UINT64_C(3));

    uint32_t acc = (uint32_t)(challenge >> 6)
                   ^ (uint32_t)(challenge >> 32)
                   ^ UINT32_C(0x95b0);

    int node = heads[index];

    for (unsigned i = 0; i <= steps; ++i) {
        if (node < 0 || node >= 12) return 1;

        acc += field12[node];
        acc *= UINT32_C(5);
        acc ^= field8[node];
        values[1] = field8[node];

        if (i < steps) {
            node = next[node];
        }
    }

    values[0] = acc;
    return write_answer(out, cap, values, count);
}

static uint32_t phase6_apply(unsigned index, uint32_t x) {
    uint32_t a;
    uint32_t b;

    switch (index) {
        case 0:  /* p6_op_3 */
            return (x >> 2) | (x << 30);

        case 1:  /* p6_op_2 */
            a = x * UINT32_C(2) + UINT32_C(0x4153);
            b = x * UINT32_C(5);
            return a ^ b;

        case 2:
        case 7:  /* p6_op_5 */
            a = x + UINT32_C(0x4400);
            return a ^ (x >> 1);

        case 3:  /* p6_op_4 */
            a = (x >> 7) ^ x;
            return a + UINT32_C(0x14a2);

        case 4:
        case 5:  /* p6_op_0 */
            a = (x << 7) | (x >> 25);
            return a ^ UINT32_C(0xea5b);

        case 6:  /* p6_op_1 */
            a = x ^ UINT32_C(0x2a2);
            b = x << 5;
            return a + b;

        default:
            return 0;
    }
}
int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32);

    unsigned first =
        (unsigned)(challenge & UINT64_C(7));
    unsigned second =
        (unsigned)((challenge >> 3) & UINT64_C(7));

    uint32_t result = phase6_apply(first, x);
    result ^= UINT32_C(0xfa82);
    result = phase6_apply(second, result);
    result += UINT32_C(0xef67);

    values[0] = result;
    return write_answer(out, cap, values, count);
}


int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t c = (uint32_t)challenge
                 ^ (uint32_t)(challenge >> 32);
    uint32_t k = (uint32_t)context
                 ^ (uint32_t)(context >> 32);
    uint32_t n = (uint32_t)(context & UINT64_C(3));

    uint32_t x = c ^ k;

    uint32_t mixed = (x >> 3) ^ UINT32_C(0xf1f8);
    mixed += x ^ UINT32_C(0xb047);

    uint32_t extra = n * UINT32_C(0xb047);
    extra ^= (uint32_t)(context >> 16);
    uint32_t second = mixed ^ extra;

    uint32_t tail = k + n;
    tail += UINT32_C(0xf1f8);
    tail += c >> 3;
    uint32_t third = second ^ tail;

    int size = snprintf(
        out, cap,
        "AAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAA"
        "OPEN"
        "%" PRIu32 " %" PRIu32 " %" PRIu32,
        n, second, third
    );

    return size < 0 || (size_t)size >= cap;
}
