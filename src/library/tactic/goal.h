/*
Copyright (c) 2013-2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once
#include <utility>
#include "util/lua.h"
#include "util/list.h"
#include "util/name.h"
#include "kernel/formatter.h"
#include "kernel/environment.h"
#include "library/io_state_stream.h"

namespace lean {
/**
   \brief A goal is just encoding the synthesis problem <tt>(?m l_1 .... l_n) : t</tt>
   That is, we want to find a term \c ?m s.t. <tt>(?m l_1 ... l_n)</tt> has type \c t
   The terms \c l_i are just local constants.

   We can convert any metavariable
       <tt>?m : Pi (x_1 : A_1) ... (x_n : A_n[x_1, ..., x_{n-1}]), B[x_1, ..., x_n]</tt>
   into a goal by simply creating the local constants
      <tt>l_1 : A_1, ..., l_n : A_n[l_1, ..., l_{n-1}]</tt>
   and then, create the goal
      <tt>?m l_1 ... l_n : B[l_1, ..., l_n]</tt>
   Now, suppose we find a term \c b with type <tt>B[l_1, ... l_n]</tt>, then we can simply
   find \c ?m by abstracting <tt>l_1, ..., l_n</tt>

   We can check whether a goal is well formed in an environment by type checking.
*/
class goal {
    expr    m_meta;
    expr    m_type;
public:
    goal() {}
    goal(expr const & m, expr const & t):m_meta(m), m_type(t) {}

    expr const & get_meta() const { return m_meta; }
    expr const & get_type() const { return m_type; }

    name get_name() const { return mlocal_name(get_app_fn(m_meta)); }

    expr get_mvar() const { return get_app_fn(m_meta); }

    /**
        \brief Given a term \c v with type get_type(), build a lambda abstraction
        that is the solution for the metavariable associated with this goal.
    */
    expr abstract(expr const & v) const;

    /**
        \brief Create a metavariable application <tt>(?m l_1 ... l_n)</tt> with the given type,
        and the locals from this goal. If <tt>only_contextual == true</tt>, then we only include
        the local constants that are marked as contextual.
    */
    expr mk_meta(name const & m, expr const & type, bool only_contextual = true) const;

    /**
        brief Return true iff get_type() only contains local constants that arguments
        of get_meta(), and each argument of get_meta() only contains local constants
        that are previous arguments.
    */
    bool validate_locals() const;

    /**
        \brief Return true iff \c validate_locals() return true and type of \c get_meta() in
        \c env is get_type()
    */
    bool validate(environment const & env) const;

    format pp(formatter const & fmt) const;
};

io_state_stream const & operator<<(io_state_stream const & out, goal const & g);

UDATA_DEFS(goal)
void open_goal(lua_State * L);
}
