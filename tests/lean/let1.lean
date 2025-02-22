-- Correct version
check let bool [inline]              := Type.{0},
          and  [inline] (p q : bool) := ∀ c : bool, (p → q → c) → c,
          infixl `∧`:25             := and,
          and_intro (p q : bool) (H1 : p) (H2 : q) : p ∧ q
              := λ (c : bool) (H : p → q → c), H H1 H2,
          and_elim_left  (p q : bool) (H : p ∧ q) : p
              := H p (λ (H1 : p) (H2 : q), H1),
          and_elim_right (p q : bool) (H : p ∧ q) : q
              := H q (λ (H1 : p) (H2 : q), H2)
       in and_intro

check let bool [inline]                := Type.{0},
          and  [inline] (p q : bool)   := ∀ c : bool, (p → q → c) → c,
          infixl `∧`:25               := and,
          and_intro [fact] (p q : bool) (H1 : p) (H2 : q) : q ∧ p
              := λ (c : bool) (H : p → q → c), H H1 H2,
          and_elim_left  (p q : bool) (H : p ∧ q) : p
              := H p (λ (H1 : p) (H2 : q), H1),
          and_elim_right (p q : bool) (H : p ∧ q) : q
              := H q (λ (H1 : p) (H2 : q), H2)
       in and_intro

