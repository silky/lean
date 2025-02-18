/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once
#include <algorithm>
#include "util/lazy_list.h"
#include "util/list.h"
#include "util/name_generator.h"
#include "kernel/expr.h"
#include "kernel/justification.h"
#include "kernel/metavar.h"

namespace lean {
/**
   \brief The lean kernel type checker produces two kinds of constraints:

   - Equality constraint:          t ≈ s
        The terms t and s must be definitionally equal.
   - Universe level constaint:     l = m
        The universe level l must be less than or equal to m.

   \remark The constraints are only generated if the input term contains
   metavariables or level metavariables.

   Each constraint is associated with a justification object.

   \remark We also have choice constraints that are used by elaborator to specify
   the possible solutions for a metavariable. The choice constraints are not used by
   the kernel.
*/
enum class constraint_kind { Eq, LevelEq, Choice };
class constraint;
typedef list<constraint> constraints;
/**
   \brief A choice_fn is used to enumerate the possible solutions for a metavariable.
   The input arguments are:
        - metavariable that should be inferred
        - the metavariable type
        - substitution map (metavar -> value)
        - name generator
   The result is a lazy_list of constraints

   One application of choice constraints is overloaded notation.
*/
typedef std::function<lazy_list<constraints>(expr const &, expr const &, substitution const &, name_generator const &)> choice_fn;

struct constraint_cell;
class constraint {
    constraint_cell * m_ptr;
    constraint(constraint_cell * ptr);
public:
    constraint(constraint const & c);
    constraint(constraint && s);
    ~constraint();

    constraint_kind kind() const;
    justification const & get_justification() const;

    constraint & operator=(constraint const & c);
    constraint & operator=(constraint && c);

    friend bool is_eqp(constraint const & c1, constraint const & c2) { return c1.m_ptr == c2.m_ptr; }
    friend void swap(constraint & l1, constraint & l2) { std::swap(l1, l2); }

    friend constraint mk_eq_cnstr(expr const & lhs, expr const & rhs, justification const & j, bool relax_main_opaque);
    friend constraint mk_level_eq_cnstr(level const & lhs, level const & rhs, justification const & j);
    friend constraint mk_choice_cnstr(expr const & m, choice_fn const & fn, unsigned delay_factor, justification const & j,
                                      bool relax_main_opaque);

    constraint_cell * raw() const { return m_ptr; }
};

inline bool operator==(constraint const & c1, constraint const & c2) { return c1.raw() == c2.raw(); }
inline bool operator!=(constraint const & c1, constraint const & c2) { return !(c1 == c2); }

/**
   \brief Create a unification constraint lhs =?= rhs
   If \c relax_main_opaque is true, then opaque definitions from the main module are treated as transparent.
*/
constraint mk_eq_cnstr(expr const & lhs, expr const & rhs, justification const & j, bool relax_main_opaque);
constraint mk_level_eq_cnstr(level const & lhs, level const & rhs, justification const & j);
constraint mk_choice_cnstr(expr const & m, choice_fn const & fn, unsigned delay_factor, justification const & j, bool relax_main_opaque);

inline bool is_eq_cnstr(constraint const & c) { return c.kind() == constraint_kind::Eq; }
inline bool is_level_eq_cnstr(constraint const & c) { return c.kind() == constraint_kind::LevelEq; }
inline bool is_choice_cnstr(constraint const & c) { return c.kind() == constraint_kind::Choice; }

constraint update_justification(constraint const & c, justification const & j);

/** \brief Return the lhs of an equality constraint. */
expr const & cnstr_lhs_expr(constraint const & c);
/** \brief Return the rhs of an equality constraint. */
expr const & cnstr_rhs_expr(constraint const & c);
/** \brief Return true iff opaque definitions from the main module should be treated as transparent. */
bool relax_main_opaque(constraint const & c);
/** \brief Return the lhs of an level constraint. */
level const & cnstr_lhs_level(constraint const & c);
/** \brief Return the rhs of an level constraint. */
level const & cnstr_rhs_level(constraint const & c);
/** \brief Return the expression associated with a choice constraint */
expr const & cnstr_expr(constraint const & c);
/** \brief Return the choice_fn associated with a choice constraint. */
choice_fn const & cnstr_choice_fn(constraint const & c);
/** \brief Return the choice constraint delay factor */
unsigned cnstr_delay_factor(constraint const & c);

/** \brief Printer for debugging purposes */
std::ostream & operator<<(std::ostream & out, constraint const & c);
}
