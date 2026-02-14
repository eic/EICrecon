((c++-mode . ((eval . (progn
;; Check if the package is installed; if not, try to load it
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
