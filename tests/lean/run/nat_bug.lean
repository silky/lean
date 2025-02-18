import logic decidable
using decidable

inductive nat : Type :=
| zero : nat
| succ : nat → nat

theorem induction_on {P : nat → Prop} (a : nat) (H1 : P zero) (H2 : ∀ (n : nat) (IH : P n), P (succ n)) : P a
:= nat_rec H1 H2 a

definition pred (n : nat) := nat_rec zero (fun m x, m) n
theorem pred_zero : pred zero = zero := refl _
theorem pred_succ (n : nat) : pred (succ n) = n := refl _

theorem zero_or_succ (n : nat) : n = zero ∨ n = succ (pred n)
:= induction_on n
    (or_intro_left _ (refl zero))
    (take m IH, or_intro_right _
      (show succ m = succ (pred (succ m)), from congr2 succ (symm (pred_succ m))))

theorem zero_or_succ2 (n : nat) : n = zero ∨ n = succ (pred n)
:= @induction_on _ n
    (or_intro_left _ (refl zero))
    (take m IH, or_intro_right _
      (show succ m = succ (pred (succ m)), from congr2 succ (symm (pred_succ m))))
