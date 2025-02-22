/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <string>
#include "kernel/error_msgs.h"

namespace lean {
format pp_indent_expr(formatter const & fmt, expr const & e) {
    return nest(get_pp_indent(fmt.get_options()), compose(line(), fmt(e)));
}

format pp_type_expected(formatter const & fmt, expr const & e) {
    return compose(format("type expected at"), pp_indent_expr(fmt, e));
}

format pp_function_expected(formatter const & fmt, expr const & e) {
    return compose(format("function expected at"), pp_indent_expr(fmt, e));
}

MK_THREAD_LOCAL_GET_DEF(list<options>, get_distinguishing_pp_options)

list<options> set_distinguishing_pp_options(list<options> const & opts) {
    list<options> r = get_distinguishing_pp_options();
    get_distinguishing_pp_options() = opts;
    return r;
}

static std::tuple<format, format> pp_until_different(formatter const & fmt, expr const & e1, expr const & e2, list<options> extra) {
    formatter fmt1 = fmt;
    while (true) {
        format r1 = pp_indent_expr(fmt1, e1);
        format r2 = pp_indent_expr(fmt1, e2);
        if (!format_pp_eq(r1, r2, fmt1.get_options()))
            return mk_pair(r1, r2);
        if (!extra)
            return mk_pair(pp_indent_expr(fmt, e1), pp_indent_expr(fmt, e2));
        options o = join(head(extra), fmt.get_options());
        fmt1  = fmt.update_options(o);
        extra = tail(extra);
    }
}

std::tuple<format, format> pp_until_different(formatter const & fmt, expr const & e1, expr const & e2) {
    return pp_until_different(fmt, e1, e2, get_distinguishing_pp_options());
}

format pp_app_type_mismatch(formatter const & fmt, expr const & app, expr const & arg, expr const & expected_type, expr const & given_type) {
    format r;
    format expected_fmt, given_fmt;
    std::tie(expected_fmt, given_fmt) = pp_until_different(fmt, expected_type, given_type);
    r += format("type mismatch at application");
    r += pp_indent_expr(fmt, app);
    r += compose(line(), format("term"));
    r += pp_indent_expr(fmt, arg);
    r += compose(line(), format("is expected of type"));
    r += expected_fmt;
    r += compose(line(), format("but is given type"));
    r += given_fmt;
    return r;
}

format pp_def_type_mismatch(formatter const & fmt, name const & n, expr const & expected_type, expr const & given_type) {
    format expected_fmt, given_fmt;
    std::tie(expected_fmt, given_fmt) = pp_until_different(fmt, expected_type, given_type);
    format r("type mismatch at definition '");
    r += format(n);
    r += format("', it is expected of type");
    r += expected_fmt;
    r += compose(line(), format("but is given type"));
    r += given_fmt;
    return r;
}

static format pp_until_meta_visible(formatter const & fmt, expr const & e, list<options> extra) {
    formatter fmt1 = fmt;
    while (true) {
        format r = pp_indent_expr(fmt1, e);
        std::ostringstream out;
        out << mk_pair(r, fmt1.get_options());
        if (out.str().find("?M") != std::string::npos)
            return r;
        if (!extra)
            return pp_indent_expr(fmt, e);
        options o = join(head(extra), fmt.get_options());
        fmt1  = fmt.update_options(o);
        extra = tail(extra);
    }
}

format pp_until_meta_visible(formatter const & fmt, expr const & e) {
    return pp_until_meta_visible(fmt, e, get_distinguishing_pp_options());
}

format pp_decl_has_metavars(formatter const & fmt, name const & n, expr const & e, bool is_type) {
    format r("failed to add declaration '");
    r += format(n);
    r += format("' to environment, ");
    if (is_type)
        r += format("type");
    else
        r += format("value");
    r += format(" has metavariables");
    r += pp_until_meta_visible(fmt, e);
    return r;
}
}
