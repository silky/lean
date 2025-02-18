/*
Copyright (c) 2013 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <iostream>
#include "util/test.h"
#include "library/max_sharing.h"
#include "kernel/abstract.h"
using namespace lean;

static void tst1() {
    max_sharing_fn max_fn;
    expr a1 = Const("a");
    expr a2 = Const("a");
    expr N = Const("N");
    expr x = Local("x", N);
    expr y = Local("y", N);
    expr f = Const("f");
    expr F1, F2;
    F1 = f(Fun(x, f(x, x)), Fun(y, f(y, y)));
    lean_assert(!is_eqp(app_arg(app_fn(F1)), app_arg(F1)));
    F2 = max_fn(F1);
    std::cout << F2 << "\n";
    lean_assert(is_eqp(app_arg(app_fn(F2)), app_arg(F2)));
    max_fn.clear();
}

static void tst2() {
    max_sharing_fn max_fn1;
    max_sharing_fn max_fn2;
    expr x   = Const("x");
    expr f   = Const("f");
    expr g   = Const("g");
    expr t1  = max_fn2(max_fn1(f(g(x))));
    expr t2  = max_fn2(f(t1, g(x)));
    expr arg1 = app_arg(app_arg(app_fn(t2)));
    expr arg2 = app_arg(t2);
    lean_assert(is_eqp(arg1, arg2));
}

static void tst3() {
    max_sharing_fn max_fn;
    expr a1 = Const("a");
    expr a2 = Const("a");
    expr a3 = Const("a");
    expr g  = Const("g");
    expr f  = Const("f");
    expr new_a1 = max_fn(a1);
    lean_assert(is_eqp(new_a1, a1));
    expr t1 = max_fn(f(a2));
    lean_assert(is_eqp(app_arg(t1), a1));
    expr t2 = max_fn(f(a2));
    lean_assert(is_eqp(t1, t2));
    expr t3 = max_fn(f(a3));
    lean_assert(is_eqp(t1, t3));
}

int main() {
    save_stack_info();
    scoped_expr_caching set(false);
    tst1();
    tst2();
    tst3();
    return has_violations() ? 1 : 0;
}
