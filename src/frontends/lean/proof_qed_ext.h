/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once
#include "kernel/environment.h"
#include "frontends/lean/cmd_table.h"

namespace lean {
environment add_proof_qed_pre_tactic(environment const & env, expr const & pre_tac);
environment reset_proof_qed_pre_tactic(environment const & env, expr const & pre_tac);
optional<expr> get_proof_qed_pre_tactic(environment const & env);
void register_proof_qed_cmds(cmd_table & r);
}
