/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "util/test.h"
#include "kernel/abstract.h"
#include "library/head_map.h"
using namespace lean;

static void tst1() {
    head_map<expr> map;
    expr a = Const("a");
    expr f = Const("f");
    expr b = Const("b");
    expr x = Local("x", Prop);
    expr l1 = Fun(x, x);
    expr l2 = Fun(x, f(x));
    lean_assert(l1 != l2);
    lean_assert(map.empty());
    map.insert(a, a);
    lean_assert(map.contains(a));
    map.insert(a, b);
    lean_assert(map.contains(a));
    map.erase_entry(a, b);
    lean_assert(map.contains(a));
    map.erase_entry(a, a);
    lean_assert(!map.contains(a));
    lean_assert(map.empty());
    map.insert(l1, a);
    map.insert(l2, b);
    lean_assert(map.contains(l1));
    lean_assert(length(*map.find(l1)) == 2);
    auto size_fn = [](head_map<expr> const & m) {
        unsigned r = 0;
        m.for_each_entry([&](head_index const &, expr const &) { r++; });
        return r;
    };
    lean_assert(size_fn(map) == 2);
    map.insert(a, a);
    lean_assert(size_fn(map) == 3);
    map.erase(l1);
    lean_assert(size_fn(map) == 1);
    map.insert(a, b);
    lean_assert(size_fn(map) == 2);
    map.erase(a);
    lean_assert(size_fn(map) == 0);
    lean_assert(map.empty());
    map.insert(f(a), f(a));
    map.insert(f(b), f(b));
    lean_assert(length(*map.find(f(a))) == 2);
}

int main() {
    save_stack_info();
    tst1();
    return has_violations() ? 1 : 0;
}
