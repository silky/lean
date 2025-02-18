import standard
using tactic

definition basic_tac : tactic
:= repeat [apply @and_intro | assumption]

set_proof_qed basic_tac -- basic_tac is automatically applied to each element of a proof-qed block

theorem tst (a b : Prop) (H : ¬ a ∨ ¬ b) (Hb : b) : ¬ a ∧ b :=
proof
  assume Ha, or_elim H
    (assume Hna, absurd Ha Hna)
    (assume Hnb, absurd Hb Hnb)
qed