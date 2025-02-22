import standard

namespace setoid
  inductive setoid : Type :=
  | mk_setoid: Π (A : Type'), (A → A → Prop) → setoid

  set_option pp.universes true

  check setoid
  definition test : Type.{2} := setoid.{0}

  definition carrier (s : setoid)
  := setoid_rec (λ a eq, a) s

  definition eqv {s : setoid} : carrier s → carrier s → Prop
  := setoid_rec (λ a eqv, eqv) s

  infix `≈`:50 := eqv

  coercion carrier

  inductive morphism (s1 s2 : setoid) : Type :=
  | mk_morphism : Π (f : s1 → s2), (∀ x y, x ≈ y → f x ≈ f y) → morphism s1 s2

  check mk_morphism
  check λ (s1 s2 : setoid), s1
  check λ (s1 s2 : Type), s1

  inductive morphism2 (s1 : setoid) (s2 : setoid) : Type :=
  | mk_morphism2 : Π (f : s1 → s2), (∀ x y, x ≈ y → f x ≈ f y) → morphism2 s1 s2

  check morphism2
  check mk_morphism2
end