/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <string>
#include "util/sstream.h"
#include "library/scoped_ext.h"
#include "library/kernel_serializer.h"
#include "frontends/lean/parser.h"

namespace lean {
struct class_entry {
    name  m_class;
    name  m_instance;
    class_entry() {}
    class_entry(name const & c, name const & i):m_class(c), m_instance(i) {}
};

struct class_state {
    typedef rb_map<name, list<name>, name_quick_cmp> class_instances;
    class_instances m_instances;
    void add_instance(name const & c, name const & i) {
        auto it = m_instances.find(c);
        if (!it)
            m_instances.insert(c, list<name>(i));
        else
            m_instances.insert(c, cons(i, filter(*it, [&](name const & i1) { return i1 != i; })));
    }
};

struct class_config {
    typedef class_state state;
    typedef class_entry entry;
    static void add_entry(environment const &, io_state const &, state & s, entry const & e) {
        s.add_instance(e.m_class, e.m_instance);
    }
    static name const & get_class_name() {
        static name g_class_name("class");
        return g_class_name;
    }
    static std::string const & get_serialization_key() {
        static std::string g_key("class");
        return g_key;
    }
    static void  write_entry(serializer & s, entry const & e) {
        s << e.m_class << e.m_instance;
    }
    static entry read_entry(deserializer & d) {
        entry e;
        d >> e.m_class >> e.m_instance;
        return e;
    }
};

template class scoped_ext<class_config>;
typedef scoped_ext<class_config> class_ext;

name get_class_name(environment const & env, expr const & e) {
    if (!is_constant(e))
        throw exception("class expected, expression is not a constant");
    name const & c_name = const_name(e);
    declaration c_d = env.get(c_name);
    if (c_d.is_definition() && !c_d.is_opaque())
        throw exception(sstream() << "invalid class, '" << c_name << "' is a transparent definition");
    return c_name;
}

environment add_instance(environment const & env, name const & n) {
    declaration d = env.get(n);
    expr type = d.get_type();
    while (is_pi(type))
        type = binding_body(type);
    name c = get_class_name(env, get_app_fn(type));
    return class_ext::add_entry(env, get_dummy_ios(), class_entry(c, n));
}

bool is_class(environment const & env, name const & c) {
    class_state const & s = class_ext::get_state(env);
    return s.m_instances.contains(c);
}

list<name> get_class_instances(environment const & env, name const & c) {
    class_state const & s = class_ext::get_state(env);
    if (auto it = s.m_instances.find(c))
        return *it;
    else
        return list<name>();
}

environment add_instance_cmd(parser & p) {
    bool found = false;
    environment env = p.env();
    while (p.curr_is_identifier()) {
        found    = true;
        name c   = p.check_constant_next("invalid 'class instance' declaration, constant expected");
        env = add_instance(env, c);
    }
    if (!found)
        throw parser_error("invalid 'class instance' declaration, at least one identifier expected", p.pos());
    return env;
}

void register_class_cmds(cmd_table & r) {
    add_cmd(r, cmd_info("instance", "add a new instance", add_instance_cmd));
}
}
