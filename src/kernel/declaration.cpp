/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "kernel/declaration.h"
#include "kernel/environment.h"
#include "kernel/for_each_fn.h"

namespace lean {
struct declaration::cell {
    MK_LEAN_RC();
    name              m_name;
    level_param_names m_params;
    expr              m_type;
    bool              m_theorem;
    optional<expr>    m_value;        // if none, then declaration is actually a postulate
    // The following fields are only meaningful for definitions (which are not theorems)
    unsigned          m_weight;
    unsigned          m_module_idx;   // module idx where it was defined
    bool              m_opaque;
    // The following field affects the convertability checker.
    // Let f be this definition, then if the following field is true,
    // then whenever we are checking whether
    //    (f a) is convertible to (f b)
    // we will first check whether a is convertible to b.
    // If the test fails, then we perform the full check.
    bool              m_use_conv_opt;
    void dealloc() { delete this; }

    cell(name const & n, level_param_names const & params, expr const & t, bool is_axiom):
        m_rc(1), m_name(n), m_params(params), m_type(t), m_theorem(is_axiom),
        m_weight(0), m_module_idx(0), m_opaque(true), m_use_conv_opt(false) {}
    cell(name const & n, level_param_names const & params, expr const & t, bool is_thm, expr const & v,
         bool opaque, unsigned w, module_idx mod_idx, bool use_conv_opt):
        m_rc(1), m_name(n), m_params(params), m_type(t), m_theorem(is_thm),
        m_value(v), m_weight(w), m_module_idx(mod_idx), m_opaque(opaque), m_use_conv_opt(use_conv_opt) {}
};

declaration g_dummy = mk_axiom(name(), level_param_names(), expr());

declaration::declaration():declaration(g_dummy) {}
declaration::declaration(cell * ptr):m_ptr(ptr) {}
declaration::declaration(declaration const & s):m_ptr(s.m_ptr) { if (m_ptr) m_ptr->inc_ref(); }
declaration::declaration(declaration && s):m_ptr(s.m_ptr) { s.m_ptr = nullptr; }
declaration::~declaration() { if (m_ptr) m_ptr->dec_ref(); }

declaration & declaration::operator=(declaration const & s) { LEAN_COPY_REF(s); }
declaration & declaration::operator=(declaration && s) { LEAN_MOVE_REF(s); }

bool declaration::is_definition() const { return static_cast<bool>(m_ptr->m_value); }
bool declaration::is_var_decl() const   { return !is_definition(); }
bool declaration::is_axiom() const      { return is_var_decl() && m_ptr->m_theorem; }
bool declaration::is_theorem() const    { return is_definition() && m_ptr->m_theorem; }

name declaration::get_name() const { return m_ptr->m_name; }
level_param_names const & declaration::get_univ_params() const { return m_ptr->m_params; }
expr declaration::get_type() const { return m_ptr->m_type; }

bool declaration::is_opaque() const { return m_ptr->m_opaque; }
expr declaration::get_value() const { lean_assert(is_definition()); return *(m_ptr->m_value); }
unsigned declaration::get_weight() const { return m_ptr->m_weight; }
module_idx declaration::get_module_idx() const { return m_ptr->m_module_idx; }
bool declaration::use_conv_opt() const { return m_ptr->m_use_conv_opt; }

declaration mk_definition(name const & n, level_param_names const & params, expr const & t, expr const & v,
                          bool opaque, unsigned weight, module_idx mod_idx, bool use_conv_opt) {
    return declaration(new declaration::cell(n, params, t, false, v, opaque, weight, mod_idx, use_conv_opt));
}
declaration mk_definition(environment const & env, name const & n, level_param_names const & params, expr const & t, expr const & v,
                          bool opaque, module_idx mod_idx, bool use_conv_opt) {
    unsigned w = 0;
    for_each(v, [&](expr const & e, unsigned) {
            if (is_constant(e)) {
                auto d = env.find(const_name(e));
                if (d && d->get_weight() > w)
                    w = d->get_weight();
            }
            return true;
        });
    return mk_definition(n, params, t, v, opaque, w+1, mod_idx, use_conv_opt);
}
declaration mk_theorem(name const & n, level_param_names const & params, expr const & t, expr const & v, module_idx mod_idx) {
    return declaration(new declaration::cell(n, params, t, true, v, true, 0, mod_idx, false));
}
declaration mk_axiom(name const & n, level_param_names const & params, expr const & t) {
    return declaration(new declaration::cell(n, params, t, true));
}
declaration mk_var_decl(name const & n, level_param_names const & params, expr const & t) {
    return declaration(new declaration::cell(n, params, t, false));
}
}
