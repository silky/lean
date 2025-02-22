import standard
using tactic

theorem tst {A B : Prop} (H1 : A) (H2 : B) : ((fun x : Prop, x) A) ∧ B ∧ A
:= by apply and_intro; beta; assumption; apply and_intro; !assumption

