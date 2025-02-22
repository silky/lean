local env = environment()
local A = Local("A", Type)
env = add_decl(env, mk_var_decl("eq", Pi(A, mk_arrow(A, A, Prop))))
local eq = Const("eq")
local a  = Local("a", A)
local b  = Local("b", A)
local H  = Local("H", eq(A, a, b))
local t  = eq(A, b, a)
local m  = mk_metavar("m", Pi(A, a, b, H, t))(A, a, b, H)
print(m)
print(env:type_check(m))
local g  = goal(m, t)
assert(g:validate(env))
local m1 = g:mk_meta("m1", Prop)
print(tostring(m1))
print(env:type_check(m1))
