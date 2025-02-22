import logic

abbreviation subsets (P : Type) := P → Prop.

section

hypothesis A : Type.

hypothesis r : A → subsets A.

hypothesis i : subsets A → A.

hypothesis retract {P : subsets A} {a : A} : r (i P) a = P a.

definition delta (a:A) : Prop := ¬ (r a a).

notation `δ` := delta.

theorem delta_aux : ¬ (δ (i delta))
         := assume H : δ (i delta),
            H (subst (symm retract) H).

check delta_aux.