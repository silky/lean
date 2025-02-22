import standard

inductive nat : Type :=
| zero : nat
| succ : nat → nat

definition add (x y : nat)
:= nat_rec x (λ n r, succ r) y

infixl `+`:65 := add

theorem add_zero_left (x : nat) : x + zero = x
:= refl _

theorem add_succ_left (x y : nat) : x + (succ y) = succ (x + y)
:= refl _

definition is_zero (x : nat)
:= nat_rec true (λ n r, false) x

theorem is_zero_zero : is_zero zero
:= eqt_elim (refl _)

theorem not_is_zero_succ (x : nat) : ¬ is_zero (succ x)
:= eqf_elim (refl _)

theorem dichotomy (m : nat) : m = zero ∨ (∃ n, m = succ n)
:= nat_rec
     (or_intro_left _ (refl zero))
     (λ m H, or_intro_right _ (exists_intro m (refl (succ m))))
     m

theorem is_zero_to_eq (x : nat) (H : is_zero x) : x = zero
:= or_elim (dichotomy x)
     (assume Hz : x = zero, Hz)
     (assume Hs : (∃ n, x = succ n),
       exists_elim Hs (λ (w : nat) (Hw : x = succ w),
         absurd_elim _ H (subst (symm Hw) (not_is_zero_succ w))))

theorem is_not_zero_to_eq {x : nat} (H : ¬ is_zero x) : ∃ n, x = succ n
:= or_elim (dichotomy x)
     (assume Hz : x = zero,
       absurd_elim _ (subst (symm Hz) is_zero_zero) H)
     (assume Hs, Hs)

theorem not_zero_add (x y : nat) (H : ¬ is_zero y) : ¬ is_zero (x + y)
:= exists_elim (is_not_zero_to_eq H)
     (λ (w : nat) (Hw : y = succ w),
        have H1 : x + y = succ (x + w), from
          calc x + y = x + succ w   : {Hw}
                 ... = succ (x + w) : refl _,
        have H2 : ¬ is_zero (succ (x + w)), from
          not_is_zero_succ (x+w),
        subst (symm H1) H2)

inductive not_zero (x : nat) : Prop :=
| not_zero_intro : ¬ is_zero x → not_zero x

theorem not_zero_not_is_zero {x : nat} (H : not_zero x) : ¬ is_zero x
:= not_zero_rec (λ H1, H1) H

theorem not_zero_add_right [instance] (x y : nat) (H : not_zero y) : not_zero (x + y)
:= not_zero_intro (not_zero_add x y (not_zero_not_is_zero H))

theorem not_zero_succ [instance] (x : nat) : not_zero (succ x)
:= not_zero_intro (not_is_zero_succ x)

variable div : Π (x y : nat) {H : not_zero y}, nat

variables a b : nat

set_option pp.implicit true
opaque_hint (hiding [module])
check div a (succ b)
check (λ H : not_zero b, div a b)
check (succ zero)
check a + (succ zero)
check div a (a + (succ b))

opaque_hint (exposing [module])
check div a (a + (succ b))

opaque_hint (hiding add)
check div a (a + (succ b))

opaque_hint (exposing add)
check div a (a + (succ b))
