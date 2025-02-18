/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "kernel/inductive/inductive.h"
#include "library/inductive_unifier_plugin.h"

namespace lean {
using inductive::inductive_normalizer_extension;

/** \brief Create a HoTT (Homotopy Type Theory) compatible environment */
environment mk_hott_environment(unsigned trust_lvl) {
    environment env = environment(trust_lvl,
                                  false /* Type.{0} is proof relevant */,
                                  true  /* Eta */,
                                  false /* Type.{0} is predicative */,
                                  list<name>(name("Id")) /* Exact equality types are proof irrelevant */,
                                  /* builtin support for inductive datatypes */
                                  std::unique_ptr<normalizer_extension>(new inductive_normalizer_extension()));
    return set_unifier_plugin(env, mk_inductive_unifier_plugin());
}
}
