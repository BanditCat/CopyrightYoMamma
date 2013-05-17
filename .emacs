(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(column-number-mode t)
 '(cua-mode t nil (cua-base))
 '(explicit-bash-args (quote ("--noediting" "-l" "-i")))
 '(explicit-csh-args (quote ("-il")))
 '(fill-column 80)
 '(global-hl-line-mode t)
 '(global-mark-ring-max 1600)
 '(kill-ring-max 6000)
 '(mark-ring-max 1600)
 '(regexp-search-ring-max 1600)
 '(save-place t nil (saveplace))
 '(search-ring-max 1600)
 '(shell-file-name "E:/root/bin/bash.exe")
 '(size-indication-mode t)
 '(tool-bar-mode nil)
 '(undo-limit 800000)
 '(undo-outer-limit 120000000)
 '(undo-strong-limit 1200000)
 '(uniquify-buffer-name-style (quote forward) nil (uniquify))
 '(winner-mode t nil (winner)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(tabbar-default-face ((t (:inherit variable-pitch :background "black" :foreground "gray60" :height 0.9)))))


;; Language hooks.
(add-hook 'haskell-mode-hook 'turn-on-haskell-doc-mode)
(add-hook 'haskell-mode-hook 'turn-on-haskell-indentation)
(add-hook 'haskell-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'c-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'c++-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'emacs-lisp-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'makefile-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'lisp-interaction-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'lua-mode-hook (lambda () (flyspell-prog-mode)))
(add-hook 'fundamental-mode-hook (lambda () (flyspell-mode)))
(add-hook 'text-mode-hook (lambda () (flyspell-mode)))


;;;; This snippet enables lua-mode
;; This line is not necessary, if lua-mode.el is already on your load-path
(autoload 'lua-mode "lua-mode" "Lua editing mode." t)
(add-to-list 'auto-mode-alist '("\\.lua$" . lua-mode))
(add-to-list 'interpreter-mode-alist '("lua" . lua-mode))

(load "tabbar")

(tabbar-mode)
(setq tabbar-buffer-groups-function
      (lambda (buf)
        (with-current-buffer (get-buffer buf)
          (list (cond ((string-equal (buffer-name) "*compilation*") "code")
                      ((string-equal (buffer-name) "*shell*") "code")
                      ((string-equal (buffer-name) "Makefile") "code")
                      ((string-equal (buffer-name) "README") "codeish")
                      ((string-equal (buffer-name) "README.md") "codeish")
                      ((string-equal (buffer-name) "LICENSE") "codeish")
                      ((string-equal (buffer-name) "*Async Shell Command*") "code")
                      ((< (length (buffer-name)) 4) "other")
                      ((string-equal (substring (buffer-name) -4) ".cpp") "code")
                      ((string-equal (substring (buffer-name) -4) ".hpp") "codeish")
                      ((string-equal (substring (buffer-name) -4) ".txt") "codeish")
                      (t "other"))))))


(setq backup-directory-alist `(("." ,@(concat user-emacs-directory "backups"))))




(require 'color-theme)
(color-theme-initialize)


(setq my-color-themes color-themes)


(defun car-theme () ;figure out if we need car or caar
  (interactive)
  (cond
   ((consp (car theme-current))
    (caar theme-current))
   (t
    (car theme-current))))

(defun my-theme-set-default () ; Set the first row
  (interactive)
  (setq theme-current my-color-themes)
  (funcall (car-theme)))

(defun my-describe-theme () ; Show the current theme
  (interactive)
  (message "%s" (car-theme)))


(defun my-theme-cycle ()                
  (interactive)
  (setq theme-current (cdr theme-current))
  (if (null theme-current)
      (setq theme-current my-color-themes)
    (funcall (car-theme)))
  (message "%S" (car-theme)))

(setq theme-current my-color-themes)
(setq color-theme-is-global nil) ; Initialization

(color-theme-billw)







(defun customBufferHelperFunction (bufferFunc)
  (let ((cb (current-buffer))
        (bn (buffer-name (current-buffer))))
    (funcall bufferFunc)  
    (while (and (not (eq cb (current-buffer)))
                buffer-read-only)
      (funcall bufferFunc))))


(defun customShortcutsHelp ()
  (interactive)
  (print "f1:help  f2:open  f3:shell  f4:next error  f5:make run or compile S-f5:make clean C-f5:make release  f6:regex search  C-M-S-f7:make backup  f8:make TAGS,make depend  f9:spell buffer  f10: format  f11:kill buffer  f12:cycle colors"))
;BUGBUG make this read a file for custom commands per directory, and do
;different things with shift ctrl etc.
(defun customRun ()
  "Load the current buffer and run it, depending on major mode."
  (interactive)
  (cond
   ((or (eq major-mode 'emacs-lisp-mode) (eq major-mode 'lisp-interaction-mode)) (eval-buffer))
   ((eq major-mode 'haskell-mode) (inferior-haskell-load-file))
   (t (compile "make run"))))
(defun customClean()
  "Run make clean."
  (interactive)
  (compile "make clean"))
(defun customRelease()
  "Run make release."
  (interactive)
  (compile "make release"))
(defun customSave ()
  "Silently save all file buffers."
  (interactive)
  (save-some-buffers 1))
(defun customHelp ()
  "Get help under point."
  (interactive)
  (if (or (eq major-mode 'emacs-lisp-mode) (eq major-mode 'lisp-interaction-mode))
      (describe-function (intern-soft (current-word)))
    (if (eq last-command 'customHelp)
        (find-tag (current-word) t)
      (find-tag-other-window (current-word))))
  (print "Press C-f1 for shotcuts" t))
(defun my-filter (condp lst)
  (delq nil
        (mapcar (lambda (x) (and (funcall condp x) x)) lst)))
(defun customSearch (str)
  (interactive "sEnter regexp to search for ")
  "Runs multi-occur on all file buffers."
  (multi-occur (my-filter (lambda (x) (> (length (buffer-file-name x)) 0)) (buffer-list)) str)
  (switch-to-buffer-other-window "*Occur*")
  (occur-edit-mode))
(defun customBuildIndices ()
  "Build help resources (e.g. TAGS file)."
  (interactive)
  (if (or (eq major-mode 'c++-mode) (eq major-mode 'c-mode))
      (progn (start-process "make" nil "make" "TAGS") 
             (start-process "make" nil "make" "depend") 
             (print "Remade TAGS and depend"))
    (print "Didn't do anything.")))
(defun customBackup ()
  "Runs make backup."
  (interactive)
  (compile "make backup"))
(defun customPreviousBuffer ()
  "Shows the previous file buffer if there is one."
  (interactive)
  (customBufferHelperFunction 'previous-buffer))
(defun customNextBuffer ()
  "Shows the previous file buffer if there is one."
  (interactive)
  (customBufferHelperFunction 'next-buffer))
(defun customNextWindow ()
  "Switches to the next visible window."
  (interactive)
  (select-window (next-window (selected-window) 1 'visible)))
(defun customPreviousWindow ()
  "Switches to the previous visible window."
  (interactive)
  (select-window (previous-window (selected-window) 1 'visible)))
;TODO make this show the prompt.
(defun customShell ()
  "Runs a real shell(tm)."
  (interactive)
  (let ((asb
         (cl-find "*Async Shell Command*" (buffer-list)
                  :test 'string-equal :key 'buffer-name)))
    (if asb (switch-to-buffer-other-window asb)
      (async-shell-command "sh -l")
      (switch-to-buffer-other-window "*Async Shell Command*"))))
(defun customFormat ()
  "Indent the whole buffer."
  (interactive)
  (indent-region (point-min-marker) (point-max-marker)))



(global-set-key (kbd "C-f") 'isearch-forward-regexp)
(global-set-key (kbd "C-S-f") 'isearch-query-replace-regexp)
(define-key isearch-mode-map (kbd "C-f") 'isearch-repeat-forward)
(define-key isearch-mode-map (kbd "C-S-f") 'isearch-query-replace-regexp)


(global-set-key (kbd "C-a") 'mark-whole-buffer) 
(global-set-key (kbd "C-s") 'save-buffer) 
(global-set-key (kbd "C-S-s") 'customSave) 
(global-set-key (kbd "<f1>") 'customHelp) 
(global-set-key (kbd "<C-f1>") 'customShortcutsHelp) 
(global-set-key (kbd "<f2>") 'find-file) 
(global-set-key (kbd "<f3>") 'customShell) 
(global-set-key (kbd "<f4>") 'next-error) 
(global-set-key (kbd "<f5>") 'customRun) 
(global-set-key (kbd "<S-f5>") 'customClean) 
(global-set-key (kbd "<C-f5>") 'customRelease) 
(global-set-key (kbd "<f6>") 'customSearch) 
(global-set-key (kbd "<C-S-M-f7>") 'customBackup) 
(global-set-key (kbd "<f8>") 'customBuildIndices)  
(global-set-key (kbd "<f9>") 'flyspell-buffer)
(global-set-key (kbd "<f10>") 'customFormat)
(global-set-key (kbd "<f11>") 'kill-buffer) 
(global-set-key (kbd "<f12>") 'my-theme-cycle)
(global-set-key (kbd "M-<up>") 'tabbar-forward-group) 
(global-set-key (kbd "M-<down>") 'tabbar-backward-group) 
(global-set-key (kbd "M-<right>") 'tabbar-forward-tab) 
(global-set-key (kbd "M-<left>") 'tabbar-backward-tab) 
(global-set-key (kbd "M-S-<right>") 'customNextWindow) 
(global-set-key (kbd "M-S-<left>") 'customPreviousWindow) 
(global-set-key (kbd "C-<right>") 'forward-sexp) 
(global-set-key (kbd "C-<left>") 'backward-sexp) 
(global-set-key (kbd "C-S-v") 'yank-pop) 
(global-set-key (kbd "C-S-<insert>") 'yank-pop) 


(desktop-save-mode 1) 


(add-hook 'window-setup-hook (lambda () (split-window-horizontally)))


(if window-system
        (add-hook 'after-init-hook
                          '(lambda()
                                 "this is executed as emacs is coming up - _after_ final-frame-params have been read from `.emacs.desktop'."
                                 (when desktop-enable
                                   (desktop-load-default)
                                   (desktop-read)
                                   ;;now size and position frame according to the  values read from disk
                                   (set-frame-size (selected-frame) (fourth final-frame-params) (fifth final-frame-params))
                                   (set-frame-position (selected-frame) (max (eval (second final-frame-params)) 0)      (max (eval (third final-frame-params)) 0))
                                   (if (sixth final-frame-params)
                                           (if (eq window-system 'w32)
                                                   (w32-send-sys-command ?\xf030)
                                                                                ;else, do X something
                                                 ))))))

(if window-system
        (add-hook 'desktop-save-hook
                          '(lambda()
                                 (let ((maximized (listp (frame-parameter (selected-frame) 'left))))
                                   "this hook sets the fram size/pos vars before `desktop.el' writes them out to disk"
                                   (if (eq window-system 'w32)
                                           (w32-send-sys-command ?\xf120)       ;restore the frame (so we can save the 'restored' size/pos)
                                                                                ;else, do X something
                                         )
                                   ;;prepend our vars to the save list so `desktop.el' will save them out to disk
                                   (setq desktop-globals-to-save (cons 'final-frame-params
                                                                                                           desktop-globals-to-save))
                                   
                                   (setq final-frame-params 
                                                 (list 
                                                  (frame-parameter (selected-frame) 'font)
                                                  (frame-parameter (selected-frame) 'left) ;x
                                                  (frame-parameter (selected-frame) 'top) ;y
                                                  (frame-width)                         ;width
                                                  (frame-height)                                ;height
                                                  maximized)))))        ;if this frame param is a list, we're probably maximized (not guaranteed)
  )


(setq c-basic-indent 2)
(setq tab-width 4)
(setq indent-tabs-mode nil)
(setq ispell-program-name "aspell")
