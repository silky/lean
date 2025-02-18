import logic

namespace N1
  section
    section
      variable A : Type
      definition foo (a : A) : Prop := true
      check foo
    end
    check foo
  end
  check foo
end
check N1.foo

namespace N2
  section
    parameter A : Type
    inductive list : Type :=
    | nil {} : list
    | cons   : A → list → list
    check list
  end
  check list
end
check N2.list
