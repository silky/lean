/*
Copyright (c) 2013 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once
#include "util/debug.h"

namespace lean {

void mix(unsigned & a, unsigned & b, unsigned & c);

unsigned hash_str(unsigned len, char const * str, unsigned init_value);

inline unsigned hash(unsigned h1, unsigned h2) {
    h2 -= h1; h2 ^= (h1 << 8);
    h1 -= h2; h2 ^= (h1 << 16);
    h2 -= h1; h2 ^= (h1 << 10);
    return h2;
}

template<typename H>
unsigned hash(unsigned n, H h, unsigned init_value = 31) {
    unsigned a, b, c;
    if (n == 0)
        return init_value;

    a = b = 0x9e3779b9;
    c = 11;

    switch (n) {
    case 1:
        a += init_value;
        b  = h(0);
        mix(a, b, c);
        return c;
    case 2:
        a += init_value;
        b += h(0);
        c += h(1);
        mix(a, b, c);
        return c;
    case 3:
        a += h(0);
        b += h(1);
        c += h(2);
        mix(a, b, c);
        a += init_value;
        mix(a, b, c);
        return c;
    default:
        while (n >= 3) {
            n--;
            a += h(n);
            n--;
            b += h(n);
            n--;
            c += h(n);
            mix(a, b, c);
        }

        a += init_value;
        switch (n) {
        case 2: b += h(1);
        case 1: c += h(0);
        }
        mix(a, b, c);
        return c;
    }
}


}
