import logic num
using num eq_proofs

inductive nat : Type :=
| zero : nat
| succ : nat → nat

abbreviation plus (x y : nat) : nat
:= nat_rec x (λn r, succ r) y
definition to_nat [coercion] [inline] (n : num) : nat
:= num_rec zero (λn, pos_num_rec (succ zero) (λn r, plus r (plus r (succ zero))) (λn r, plus r r) n) n
definition add (x y : nat) : nat
:= plus x y
variable le : nat → nat → Prop

infixl `+`:65 := add
infix `≤`:50  := le
axiom add_one (n:nat) : n + (succ zero) = succ n
axiom add_le_right_inv {n m k : nat} (H : n + k ≤ m + k) : n ≤ m

theorem succ_le_cancel {n m : nat} (H : succ n ≤ succ m) :  n ≤ m
:= add_le_right_inv (add_one m⁻¹ ▸ add_one n⁻¹ ▸ H)
