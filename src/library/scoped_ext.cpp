/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <vector>
#include <memory>
#include <string>
#include "library/scoped_ext.h"
#include "library/kernel_bindings.h"

namespace lean {
typedef std::tuple<name, using_namespace_fn, push_scope_fn, pop_scope_fn> entry;
typedef std::vector<entry> scoped_exts;

static scoped_exts & get_exts() {
    static std::unique_ptr<std::vector<entry>> exts;
    if (!exts.get())
        exts.reset(new std::vector<entry>());
    return *exts;
}

void register_scoped_ext(name const & c, using_namespace_fn use, push_scope_fn push, pop_scope_fn pop) {
    get_exts().emplace_back(c, use, push, pop);
}

struct scope_mng_ext : public environment_extension {
    name_set   m_namespace_set; // all namespaces registered in the system
    list<name> m_namespaces; // stack of namespaces/sections
    list<bool> m_in_section;
};

struct scope_mng_ext_reg {
    unsigned m_ext_id;
    scope_mng_ext_reg() { m_ext_id = environment::register_extension(std::make_shared<scope_mng_ext>()); }
};

static scope_mng_ext_reg g_ext;
static scope_mng_ext const & get_extension(environment const & env) {
    return static_cast<scope_mng_ext const &>(env.get_extension(g_ext.m_ext_id));
}
static environment update(environment const & env, scope_mng_ext const & ext) {
    return env.update(g_ext.m_ext_id, std::make_shared<scope_mng_ext>(ext));
}

name const & get_namespace(environment const & env) {
    scope_mng_ext const & ext = get_extension(env);
    return !is_nil(ext.m_namespaces) ? head(ext.m_namespaces) : name::anonymous();
}

list<name> const & get_namespaces(environment const & env) {
    return get_extension(env).m_namespaces;
}

bool in_section(environment const & env) {
    scope_mng_ext const & ext = get_extension(env);
    return !is_nil(ext.m_in_section) && head(ext.m_in_section);
}

environment using_namespace(environment const & env, io_state const & ios, name const & n, name const & c) {
    environment r = env;
    for (auto const & t : get_exts()) {
        if (c.is_anonymous() || c == std::get<0>(t))
            r = std::get<1>(t)(r, ios, n);
    }
    return r;
}

optional<name> to_valid_namespace_name(environment const & env, name const & n) {
    scope_mng_ext const & ext = get_extension(env);
    if (ext.m_namespace_set.contains(n))
        return optional<name>(n);
    for (auto const & ns : ext.m_namespaces) {
        name r = ns + n;
        if (ext.m_namespace_set.contains(r))
            return optional<name>(r);
    }
    return optional<name>();
}

static std::string g_new_namespace_key("nspace");
environment push_scope(environment const & env, io_state const & ios, name const & n) {
    if (!n.is_anonymous() && in_section(env))
        throw exception("invalid namespace declaration, a namespace cannot be declared inside a section");
    bool in_section = n.is_anonymous();
    name new_n = get_namespace(env) + n;
    scope_mng_ext ext = get_extension(env);
    bool save_ns = false;
    if (!ext.m_namespace_set.contains(new_n)) {
        save_ns  = true;
        ext.m_namespace_set.insert(new_n);
    }
    ext.m_namespaces = list<name>(new_n, ext.m_namespaces);
    ext.m_in_section = list<bool>(n.is_anonymous(), ext.m_in_section);
    environment r = update(env, ext);
    for (auto const & t : get_exts()) {
        r = std::get<2>(t)(r, in_section);
    }
    if (!n.is_anonymous())
        r = using_namespace(r, ios, n);
    if (save_ns)
        r = module::add(r, g_new_namespace_key, [=](serializer & s) { s << new_n; });
    return r;
}

static void namespace_reader(deserializer & d, module_idx, shared_environment &,
                             std::function<void(asynch_update_fn const &)> &,
                             std::function<void(delayed_update_fn const &)> & add_delayed_update) {
    name n;
    d >> n;
    add_delayed_update([=](environment const & env, io_state const &) -> environment {
            scope_mng_ext ext = get_extension(env);
            ext.m_namespace_set.insert(n);
            return update(env, ext);
        });
}
register_module_object_reader_fn g_namespace_reader(g_new_namespace_key, namespace_reader);

environment pop_scope(environment const & env) {
    scope_mng_ext ext = get_extension(env);
    if (is_nil(ext.m_namespaces))
        throw exception("invalid end of scope, there are no open namespaces/sections");
    bool in_section  = head(ext.m_in_section);
    ext.m_namespaces = tail(ext.m_namespaces);
    ext.m_in_section = tail(ext.m_in_section);
    environment r = update(env, ext);
    for (auto const & t : get_exts()) {
        r = std::get<3>(t)(r, in_section);
    }
    return r;
}

static int using_namespace_objects(lua_State * L) {
    int nargs = lua_gettop(L);
    environment const & env = to_environment(L, 1);
    name n = to_name_ext(L, 2);
    if (nargs == 2)
        return push_environment(L, using_namespace(env, get_io_state(L), n));
    else if (nargs == 3)
        return push_environment(L, using_namespace(env, get_io_state(L), n, to_name_ext(L, 3)));
    else
        return push_environment(L, using_namespace(env, to_io_state(L, 4), n, to_name_ext(L, 3)));
}

static int push_scope(lua_State * L) {
    int nargs = lua_gettop(L);
    if (nargs == 1)
        return push_environment(L, push_scope(to_environment(L, 1), get_io_state(L)));
    else if (nargs == 2)
        return push_environment(L, push_scope(to_environment(L, 1), get_io_state(L), to_name_ext(L, 2)));
    else
        return push_environment(L, push_scope(to_environment(L, 1), to_io_state(L, 3), to_name_ext(L, 2)));
}

static int pop_scope(lua_State * L) {
    return push_environment(L, pop_scope(to_environment(L, 1)));
}

void open_scoped_ext(lua_State * L) {
    SET_GLOBAL_FUN(using_namespace_objects, "using_namespace_objects");
    SET_GLOBAL_FUN(push_scope, "push_scope");
    SET_GLOBAL_FUN(pop_scope, "pop_scope");
}
}
