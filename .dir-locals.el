((c++-mode . ((eval . (progn
                         ;; 1. Set up C++ style to match .clang-format (LLVM style)

                         ;; Define LLVM style if not already defined
                         (unless (assoc "llvm" c-style-alist)
                           (c-add-style "llvm"
                                        '("gnu"
                                          (c-basic-offset . 2)
                                          (indent-tabs-mode . nil)
                                          (c-offsets-alist . ((innamespace . 0)
                                                              (arglist-intro . ++)
                                                              (member-init-intro . ++))))))

                         ;; Set default C style to LLVM (base style of .clang-format)
                         (c-set-style "llvm")
                         ;; Check if the function exists before calling to be safe
                         (if (fboundp 'c-guess)
                             (c-guess t)
                           (message "c-guess not found; using fallback llvm style."))

                         ;; 2. Set up clang-format key bindings

                         ;; Check if the clang-format package is installed; if not, try to load it
                         (unless (featurep 'clang-format)
                           (condition-case nil
                               (require 'clang-format)
                             (error (message (concat "Warning: clang-format package not found. "
                                                     "Please install it from melpa package-archive "
                                                     "at https://melpa.org/packages/ "
                                                     "with M-x package-install [RET] clang-format.")))))

                         ;; Force load the package so the functions become "commands"
                         (require 'clang-format nil t)

                         ;; Define buffer-local keybindings
                         (local-set-key (kbd "C-c i") 'clang-format-region)
                         (local-set-key (kbd "C-c u") 'clang-format-buffer)

                         ;; Optional: Ensure clang-format-buffer runs on save
                         ;; (add-hook 'before-save-hook #'clang-format-buffer nil t)
                         )))))
