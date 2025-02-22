local env = environment()
env = add_decl(env, mk_var_decl("A", Prop))
local A  = Const("A")
env = add_decl(env, mk_axiom("H1", A))
local H1 = Const("H1")
env = add_decl(env, mk_theorem("H2", A, H1))
assert(env:get("H2"):is_theorem())
env = add_decl(env, mk_definition("B", Prop, A))
env:export("mod1_mod.olean")

local env2 = import_modules("mod1_mod.olean", {keep_proofs=true})
assert(env2:get("A"):type() == Prop)
assert(env2:get("A"):is_var_decl())
assert(env2:get("H1"):type() == A)
assert(env2:get("H1"):is_axiom())
assert(env2:get("H2"):type() == A)
assert(env2:get("H2"):is_theorem())
assert(env2:get("H2"):value() == H1)
assert(env2:get("B"):type() == Prop)
assert(env2:get("B"):value() == A)
assert(env2:get("B"):is_definition())

local env3 = import_modules("mod1_mod.olean")
assert(env3:get("A"):type() == Prop)
assert(env3:get("A"):is_var_decl())
assert(env3:get("H1"):type() == A)
assert(env3:get("H1"):is_axiom())
assert(env3:get("H2"):type() == A)
assert(env3:get("H2"):is_axiom())
assert(env3:get("B"):type() == Prop)
assert(env3:get("B"):value() == A)
assert(env3:get("B"):is_definition())
