(require 'generic-x)
(require 'lean-input)
(require 'compile)
(require 'flymake)

(defvar lean-exe "lean"
  "Path for the Lean executable")

(defun flymake-create-temp-lean-in-system-tempdir (filename prefix)
  (make-temp-file (or prefix "flymake") nil ".lean"))

(defun lean-execute ()
  "Execute Lean in the current buffer"
  (interactive)
  (if (buffer-file-name)
      (compile (format "%s %s" lean-exe (buffer-file-name)))
    (compile (format "%s %s" lean-exe (flymake-init-create-temp-buffer-copy 'flymake-create-temp-lean-in-system-tempdir)))))

(defun lean-set-keys ()
  (local-set-key "\C-c\C-x" 'lean-execute)
  (local-set-key "\C-c\C-l" 'lean-execute))

(define-abbrev-table 'lean-mode-abbrev-table '(
    ("var"    "variable")
    ("vars"   "variables")
    ("def"    "definition")
    ("th"     "theorem")
    ))

(define-generic-mode
    'lean-mode     ;; name of the mode to create
  '("--")          ;; comments start with
  '("import" "abbreviation" "opaque_hint" "tactic_hint" "definition" "renaming" "inline" "hiding" "exposing" "parameter" "parameters" "proof" "qed" "conjecture" "hypothesis" "lemma" "corollary" "variable" "variables" "print" "theorem" "axiom" "inductive" "with" "universe" "alias" "help" "environment" "options" "precedence" "postfix" "prefix" "calc_trans" "calc_subst" "calc_refl" "infix" "infixl" "infixr" "notation" "eval" "check" "exit" "coercion" "end" "private" "using" "namespace" "builtin" "including" "class" "instance" "section" "set_option" "add_rewrite") ;; some keywords
  '(("\\_<\\(bool\\|int\\|nat\\|real\\|Prop\\|Type\\|ℕ\\|ℤ\\)\\_>" . 'font-lock-type-face)
    ("\\_<\\(calc\\|have\\|obtains\\|show\\|by\\|in\\|let\\|forall\\|fun\\|exists\\|if\\|then\\|else\\|assume\\|take\\|obtain\\|from\\)\\_>" . font-lock-keyword-face)
    ("\"[^\"]*\"" . 'font-lock-string-face)
    ("\\(->\\|↔\\|/\\\\\\|==\\|\\\\/\\|[*+/<=>¬∧∨≠≤≥-]\\)" . 'font-lock-constant-face)
    ("\\(λ\\|→\\|∃\\|∀\\|:\\|:=\\)" . font-lock-constant-face)
    ("\\_<\\(\\b.*_tac\\|Cond\\|or_else\\|t\\(?:hen\\|ry\\)\\|when\\|assumption\\|apply\\|b\\(?:ack\\|eta\\)\\|done\\|exact\\)\\_>" . 'font-lock-constant-face)
    ("\\_<\\(universe\\|theorem\\|axiom\\|lemma\\|hypothesis\\|abbreviation\\|definition\\|variable\\|parameter\\)\\_>[ \t\{\[]*\\([^ \t\n]*\\)" (2 'font-lock-function-name-face))
    ("\\_<\\(variables\\|parameters\\)\\_>[ \t\(\{\[]*\\([^:]*\\)" (2 'font-lock-function-name-face))
    ("\\(set_opaque\\|set_option\\)[ \t]*\\([^ \t\n]*\\)" (2 'font-lock-constant-face))
    ("\\_<_\\_>" . 'font-lock-preprocessor-face)
    ;;
    )
  '("\\.lean$")                    ;; files for which to activate this mode
  '((lambda()
      (set-input-method "Lean")
      (set (make-local-variable 'lisp-indent-function)
           'common-lisp-indent-function)
      (lean-set-keys)
      (setq local-abbrev-table lean-mode-abbrev-table)
      (abbrev-mode 1)
      ))
  "A mode for Lean files"          ;; doc string for this mode
  )

(provide 'lean-mode)

; (regexp-opt '("Int" "Bool" "Nat" "Type" "Real") t)
; (regexp-opt '("let" "in" "have" "calc" "forall" "exists") t)
; (regexp-opt '("=" "->" "≠" "∨" "∧" "¬" "/\\" "\\/" "+" "-" "*" "/" "<" ">" "≤" "≥" "==" "∀" "∃" "→" "λ" ":") t)
; (regexp-opt '("apply" "exact" "trivial" "absurd" "beta" "apply" "unfold" "done" "back" "Try" "Then" "OrElse" "OrElse" "Cond" "When" "unfold_all" ) t)
