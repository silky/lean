/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "util/lazy_list_fn.h"
#include "kernel/inductive/inductive.h"
#include "library/unifier_plugin.h"
#include "library/unifier.h"

namespace lean {
class inductive_unifier_plugin_cell : public unifier_plugin_cell {
    /** \brief Return true iff the lhs or rhs of the constraint c is of the form (elim ... (?m ...) ...) */
    bool is_elim_meta_cnstr(type_checker & tc, constraint const & c) const {
        return is_eq_cnstr(c) && (inductive::is_elim_meta_app(tc, cnstr_lhs_expr(c)) ||
                                  inductive::is_elim_meta_app(tc, cnstr_rhs_expr(c)));
    }

    /** \brief Return true iff \c e is of the form (?m ... (intro ...) ...) */
    bool is_meta_intro_app(type_checker & tc, expr const & e) const {
        if (!is_app(e) || !is_meta(e))
            return false;
        buffer<expr> args;
        get_app_args(e, args);
        for (expr const & a : args) {
            expr arg = get_app_fn(a);
            if (!is_constant(arg))
                continue;
            if (inductive::is_intro_rule(tc.env(), const_name(arg)))
                return true;
        }
        return false;
    }

    /** \brief Return true iff the lhs or rhs of the constraint c is of the form (?m ... (intro ...)) */
    bool is_meta_intro_cnstr(type_checker & tc, constraint const & c) const {
        return is_eq_cnstr(c) && (is_meta_intro_app(tc, cnstr_lhs_expr(c)) || is_meta_intro_app(tc, cnstr_rhs_expr(c)));
    }

    /**
       \brief Given (elim args) =?= t, where elim is the eliminator/recursor for the inductive declaration \c decl,
       and the major premise is of the form (?m ...), we create a case split where we try to assign (?m ...)
       to the different constructors of decl.
    */
    lazy_list<constraints> add_elim_meta_cnstrs(type_checker & tc, name_generator ngen, inductive::inductive_decl const & decl,
                                                expr const & elim, buffer<expr> & args, expr const & t, justification const & j,
                                                buffer<constraint> & tc_cnstr_buffer, bool relax) const {
        lean_assert(is_constant(elim));
        environment const & env = tc.env();
        levels elim_lvls       = const_levels(elim);
        unsigned elim_num_lvls = length(elim_lvls);
        unsigned major_idx     = *inductive::get_elim_major_idx(env, const_name(elim));
        expr meta              = args[major_idx]; // save this argument, we will update it
        lean_assert(has_expr_metavar_strict(meta));
        buffer<expr> margs;
        expr const & m     = get_app_args(meta, margs);
        expr mtype = tc.infer(m, tc_cnstr_buffer);
        lean_assert(!tc.next_cnstr());
        unsigned buff_sz = tc_cnstr_buffer.size();
        buffer<constraints> alts;
        for (auto const & intro : inductive::inductive_decl_intros(decl)) {
            tc_cnstr_buffer.shrink(buff_sz);
            name const & intro_name = inductive::intro_rule_name(intro);
            declaration intro_decl  = env.get(intro_name);
            levels intro_lvls;
            if (length(intro_decl.get_univ_params()) == elim_num_lvls) {
                intro_lvls = elim_lvls;
            } else {
                lean_assert(length(intro_decl.get_univ_params()) == elim_num_lvls - 1);
                intro_lvls = tail(elim_lvls);
            }
            expr intro_fn   = mk_constant(inductive::intro_rule_name(intro), intro_lvls);
            expr hint       = intro_fn;
            expr intro_type = tc.whnf(inductive::intro_rule_type(intro), tc_cnstr_buffer);
            while (is_pi(intro_type)) {
                hint = mk_app(hint, mk_app(mk_aux_metavar_for(ngen, mtype), margs));
                intro_type = tc.whnf(binding_body(intro_type), tc_cnstr_buffer);
                lean_assert(!tc.next_cnstr());
            }
            constraint c1      = mk_eq_cnstr(meta, hint, j, relax);
            args[major_idx]    = hint;
            lean_assert(!tc.next_cnstr());
            expr reduce_elim   = tc.whnf(mk_app(elim, args), tc_cnstr_buffer);
            lean_assert(!tc.next_cnstr());
            constraint c2      = mk_eq_cnstr(reduce_elim, t, j, relax);
            list<constraint> tc_cnstrs = to_list(tc_cnstr_buffer.begin(), tc_cnstr_buffer.end());
            alts.push_back(cons(c1, cons(c2, tc_cnstrs)));
        }
        lean_assert(!tc.next_cnstr());
        return to_lazy(to_list(alts.begin(), alts.end()));
    }

    lazy_list<constraints> process_elim_meta_core(type_checker & tc, name_generator const & ngen,
                                                  expr const & lhs, expr const & rhs, justification const & j, bool relax) const {
        lean_assert(inductive::is_elim_meta_app(tc, lhs));
        buffer<constraint> tc_cnstr_buffer;
        lean_assert(!tc.next_cnstr());
        if (!tc.is_def_eq_types(lhs, rhs, j, tc_cnstr_buffer))
            return lazy_list<constraints>();
        lean_assert(!tc.next_cnstr());
        buffer<expr> args;
        expr const & elim = get_app_args(lhs, args);
        environment const & env = tc.env();
        auto it_name = *inductive::is_elim_rule(env, const_name(elim));
        auto decls   = *inductive::is_inductive_decl(env, it_name);
        for (auto const & d : std::get<2>(decls)) {
            if (inductive::inductive_decl_name(d) == it_name)
                return add_elim_meta_cnstrs(tc, ngen, d, elim, args, rhs, j, tc_cnstr_buffer, relax);
        }
        lean_unreachable(); // LCOV_EXCL_LINE
    }

public:
    /**
       \brief Try to solve constraint of the form (elim ... (?m ...)) =?= t, by assigning (?m ...) to the introduction rules
       associated with the eliminator \c elim.
    */
    virtual lazy_list<constraints> solve(type_checker & tc, constraint const & c, name_generator const & ngen) const {
        if (!is_eq_cnstr(c))
            return lazy_list<constraints>();
        expr const & lhs        = cnstr_lhs_expr(c);
        expr const & rhs        = cnstr_rhs_expr(c);
        justification const & j = c.get_justification();
        bool relax = relax_main_opaque(c);
        if (inductive::is_elim_meta_app(tc, lhs))
            return process_elim_meta_core(tc, ngen, lhs, rhs, j, relax);
        else if (inductive::is_elim_meta_app(tc, rhs))
            return process_elim_meta_core(tc, ngen, rhs, lhs, j, relax);
        else
            return lazy_list<constraints>();
    }

    virtual bool delay_constraint(type_checker & tc, constraint const & c) const {
        return is_elim_meta_cnstr(tc, c) || is_meta_intro_cnstr(tc, c);
    }
};

unifier_plugin mk_inductive_unifier_plugin() {
    return std::make_shared<inductive_unifier_plugin_cell>();
}
}
