local env     = environment()
local l1      = param_univ("l1")
local l2      = param_univ("l2")
env = add_decl(env, mk_var_decl("functor", {l1, l2}, mk_arrow(mk_sort(l1), mk_sort(l2), mk_sort(imax_univ(l1, l2)))))
local A       = Local("A", mk_sort(l1))
local B       = Local("B", mk_sort(l2))
local functor = Const("functor", {l1, l2})
env = add_decl(env, mk_var_decl("to_fun", {l1, l2}, Pi(A, B, mk_arrow(functor(A, B), mk_arrow(A, B)))))
env = add_coercion(env, "to_fun", "functor")
for_each_coercion_fun(env, function(C, f) print(tostring(C) .. " >-> function : " .. tostring(f)) end)
env = add_decl(env, mk_var_decl("nat", Type))
env = add_decl(env, mk_var_decl("real", Type))
local nat    = Const("nat")
local real   = Const("real")
env = add_decl(env, mk_var_decl("f1", Const("functor", {1, 1})(nat, real)))
print(get_coercion_to_fun(env, Const("functor", {1, 1})(nat, real)))
env = add_decl(env, mk_var_decl("sfunctor", {l1}, mk_arrow(mk_sort(l1), mk_sort(l1))))
env = add_decl(env, mk_var_decl("sf2f", {l1}, Pi(A, mk_arrow(Const("sfunctor", {l1})(A), Const("functor", {l1, l1})(A, A)))))
env = add_coercion(env, "sf2f")
print(get_coercion_to_fun(env, Const("sfunctor", {1})(nat)))
assert(env:type_check(get_coercion_to_fun(env, Const("sfunctor", {1})(nat))))
