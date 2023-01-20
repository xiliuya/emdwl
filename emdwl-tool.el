;;; emdwl-tool.el --- summary

;; Copyright (C) 2014 xiliuya

;; Author: xiliuya <xiliuya@163.com>
;; Version: 0

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary: a so test

;;; Code:
(require 'hideshow)

(defcustom emdwl-tool-client-list nil
  "Alist of client and buffer-point
(CLIENT-ID . BUFFER-POINT)
"
  )
(defcustom emdwl-tool-client-id-list nil
  "List of client-id title appid tags.
"
  )
(defcustom emdwl-tool-client-width 200
  "Set client width."
  )
(defcustom emdwl-tool-client-height 200
  "Set client height."
  )
(defcustom emdwl-tool-time-list nil
  "List of TIME."
  )
(defun emdwl-tool-position-list-change (position-list x y height width)
  " x y height width "
  (list
   (+ (nth 0 position-list) x)
   (+ (nth 1 position-list) y)
   (+ (nth 2 position-list) height)
   (+ (nth 3 position-list) width)
   )
  )
(defun emdwl-tool-find-poistion-with-buffer-point (buffer-point)
  "Find window posiotion.
BUFFER-POINT: (BUFFER-NAME . POINT)"
  (setq buffer-obj (car buffer-point))
  (setq w-point (cdr buffer-point))
  (if (equal buffer-obj (current-buffer))
      (progn (setq w-position
                   (save-excursion
                     (goto-char w-point)
                     (setq position
                           (pos-tip-show-no-propertize "Create client"))))
             (cons (car w-position)
                   (- (cdr w-position) (line-pixel-height))))
    (null 't)
    ))

(defun emdwl-tool-open (run-cmd)
  "Open and resize.
RUN-CMD: string."
  (setq client-id (emdwl-open run-cmd))
  (sit-for 0.2)
  (emdwl-tool-resize-window (window-inside-absolute-pixel-edges ) client-id)
  (sit-for 0.2)
  (emdwl-tool-client-visible client-id)
  )
(defun emdwl-tool-open-inline (run-cmd buffer-point)
  "Open and resize.
BUFFER-POINT: (cons (current-buffer) (point))"
  (setq client-id (emdwl-open run-cmd))
  (emdwl-tool-display-inline (cdr buffer-point) client-id)
  (add-to-list 'emdwl-tool-client-list (cons client-id buffer-point))
  (sit-for 0.2)
  (emdwl-tool-resize-point client-id buffer-point)
  (sit-for 0.2)
  (emdwl-tool-client-visible client-id)
  )
(defun emdwl-tool-add-inline-client (&optional client-id)
  "Add a inline window .
CLIENT-ID: client id."
  (interactive)
  (if (null client-id)
      (setq client-id (emdwl-tool-choose-client)))
  (if (not (null client-id))
      (progn
        (setq buffer-point (cons (current-buffer) (point)))
        (emdwl-tool-display-inline (cdr buffer-point) client-id)
        (add-to-list 'emdwl-tool-client-list (cons client-id buffer-point))
        (emdwl-tool-resize-point client-id buffer-point)
        (sit-for 0.2)
        (emdwl-tool-client-visible client-id))
    )
  )
(defun emdwl-tool-resize (list client)
  "Need a list of position_size and client_id"

  (setq x (nth 0 list))
  (setq y (nth 1 list))
  (setq height (nth 2 list))
  (setq width (nth 3 list))
  (emdwl-resize x y height width client)
  )
(defun emdwl-tool-resize-window (list client)
  "Need a list of the edge pixel coordinates of WINDOW’s text area '(window-inside-absolute-pixel-edges"

  (setq x (nth 0 list))
  (setq y (nth 1 list))
  (setq height (- (nth 2 list) x))
  (setq width (- (nth 3 list) y))
  (emdwl-resize x y height width client)
  )
(defun emdwl-tool-resize-point (client buffer-point)
  "Resize with buffer-point."
  (setq inline-position
        (emdwl-tool-find-poistion-with-buffer-point buffer-point ))
  (if (not (null inline-position))
      (emdwl-resize (car inline-position)
                    (cdr inline-position)
                    emdwl-tool-client-height
                    emdwl-tool-client-width
                    client)
    )
  )

(defun emdwl-tool-new-buffer (client-title)
  "Create a client buffer"
  (buffer-name (generate-new-buffer client-title))
  )

(defun emdwl-tool-init ()
  "Init emdwl and start dwl"
  ;; (load-library "/home/xiliuya/git/xiliuya/emdwl/dwl/emdwl.so")
  (emdwl-init)
  (emdwl-run "emacsclient -cn -s emdwl")
  (when (fboundp 'tool-bar-mode)
    (tool-bar-mode -1))
  (when (fboundp 'set-scroll-bar-mode)
    (set-scroll-bar-mode nil))
  (when (fboundp 'menu-bar-mode)
    (menu-bar-mode -1))
  )
(defun emdwl-tool-start ()
  "Start a emdwl."
  (add-to-list 'load-path "./")
  (require 'emdwl)
  (require 'display-line-numbers)
  (emdwl-tool-init)
  ;;(emdwl-tool-add-advice)
  )
(defun emdwl-tool-so ()
  ;; (add-to-list 'load-path "/home/xiliuya/git/xiliuya/emdwl/dwl")
  (add-to-list 'load-path "./")
  (require 'emdwl)
  ;; (load-library "/home/xiliuya/git/xiliuya/emdwl/dwl/emdwl.so")
  (emdwl-init)
  ;;(emdwl-run "emacs")
  (emdwl-run "alacritty -e bash -c 'cmatrix -s && fish' ")
  (sit-for 1)
  (emdwl-tool-open "alacritty -e  bash -c 'echo 000 ;fish'")
  ;; (emdwl-tool-open "meld")
  ;; (emdwl-tool-open "firefox")
  ;; (emdwl-tool-open "emacs")
  ;; (emdwl-tool-open "emacsclient -cn")

  (emdwl-list)
  ;; (emdwl-resize 100 100 600 600 "Alacritty")
  ;; (sit-for 1)
  ;; (setq client0 (emdwl-open "xfce4-terminal"))
  ;; (setq client1 (emdwl-open "xfce4-terminal"))
  ;; (setq client2 (emdwl-open "xfce4-terminal"))
  ;; (setq client0 (emdwl-open "alacritty -e  bash -c 'echo 000 ;fish'"))
  ;; (sit-for 0.2)
  ;; (emdwl-resize 10 100 600 600 client0)
  ;; (sit-for 0.2)
  ;; (emdwl-newtags-client client0 0)
  ;; (sit-for 1)
  ;; (setq client1 (emdwl-open "alacritty -e  bash -c 'echo 111 ;fish'"))
  ;; (sit-for 0.2)
  ;; (emdwl-resize 20 100 600 600 client1)
  ;; (sit-for 0.2)
  ;; (emdwl-newtags-client client0 0)
  ;; (sit-for 1)
  ;; (setq client2 (emdwl-open "alacritty -e  bash -c 'echo 222 ;fish'"))
  ;; (setq client2 (emdwl-open "env WINIT_UNIX_BACKEND=x11 alacritty"))
  ;; (sit-for 0.2)
  ;; (emdwl-resize 30 100 600 600 client2)
  ;; (sit-for 0.2)
  ;; (emdwl-newtags-client client0 0)
  ;; (message "client_id: %S %S %S" client0 client1 client2)
  ;; (sit-for 1)


  ;; (sit-for 1)
  ;; (emdwl-newtags-client client0 0)
  ;; (sit-for 1)
  ;; (emdwl-newtags-client client1 0)
  ;; (sit-for 1)
  ;; (emdwl-newtags-client client2 0)
  ;; (sit-for 1)
  ;; (emdwl-newtags-client client0 1)
  ;; (sit-for 1)
  ;; (emdwl-newtags-client client1 2)
  ;; (sit-for 1)
  ;; (emdwl-newtags-client client2 3)

  ;; (emdwl-newtags-client "Alacritty" 1)
  ;; (sit-for 8)

  ;; (sit-for 2)
  ;; (emdwl-close-client client0)
  ;; (sit-for 1)
  ;; (emdwl-close-client client2)
  ;; (sit-for 1)
  ;; (emdwl-close-client client1)
  ;; (emdwl-close-client "Alacritty")
  (sit-for 10)
  (message "will close")
  (emdwl-close)
  (sit-for 10)
  (message "happy hack")
  ;;(mymod-test)
  (kill-emacs)
  )
(defun emdwl-tool-display-inline (&optional dis-point dis-client)
  "Insert a overlay for display."
  (interactive)
  (if (eq dis-point nil)
      (setq dis-point (point)))
  (if (eq dis-client nil)
      (setq dis-client 99999))

  ;; (setq position (pos-tip-show-no-propertize "Create client"))
  ;; (setq x (- (car position) 0)) ;; offset 31
  ;; (setq y (- (cdr position) (line-pixel-height)))
  ;; (message "%S %S %S %S" x y height width)

  (emdwl-tool-make-overlay dis-point dis-client)

  (cons (current-buffer) dis-point)
  )
(defun emdwl-tool-make-overlay (point-p client-id)

  (insert ">")
  (setq w-overlay (make-overlay point-p (1+ point-p) ))

  (overlay-put w-overlay 'before-string (format "\nid:%S\n-----\n" client-id))
  (setq line "\n")
  (dotimes (var (/ emdwl-tool-client-height (line-pixel-height)))
    (setq line (concat line "\n"))
    )
  (setq line (concat line "-----\n"))

  (overlay-put w-overlay 'after-string line)
  ;;(delete-overlay w-overlay)
  (emdwl-tool-find-poistion-with-buffer-point (cons (current-buffer) point-p))

  )
(defun emdwl-tool-delete-overlay (point-p)
  (setq overlay-list (overlays-at point-p))
  (dolist (w-overlay overlay-list)
    (delete-overlay w-overlay)
    )
  )
(defun emdwl-tool-is-visible-overlay (buffer-point)
  (with-current-buffer  (car buffer-point)
    (if (hs-overlay-at (cdr buffer-point))
        (null 'nil)
      (null 't)
      )
    )
  )
(defun emdwl-tool-inline-scrool-up (&optional arg)
  "Add advice after to scrool-up"
  (setq up-pixel (* arg (line-pixel-height) -1))
  (setq client-list nil)
  (dolist (client emdwl-tool-client-list)
    (add-to-list 'client-list
                 (cons (car client)
                       (emdwl-tool-position-list-change
                        (cdr client)
                        0 up-pixel 0 0))
                 )
    )
  (setq emdwl-tool-client-list client-list)
  (dolist (client emdwl-tool-client-list)
    (emdwl-tool-resize (cdr client) (car client) )
    )
  )
(defun emdwl-tool-inline-scrool-down (&optional arg)
  "Add advice after to scrool-down"
  (setq down-pixel (* arg (line-pixel-height)))
  (setq client-list nil)
  (dolist (client emdwl-tool-client-list)
    (add-to-list 'client-list
                 (cons (car client)
                       (emdwl-tool-position-list-change
                        (cdr client)
                        0 down-pixel 0 0))
                 )
    )
  (setq emdwl-tool-client-list client-list)
  (dolist (client emdwl-tool-client-list)
    (emdwl-tool-resize (cdr client) (car client))
    )
  )
(defun emdwl-tool-inline-scrool (&optional arg)
  "Add advice after to scrool-up/down"
  (emdwl-tool-client-position-update)
  )
(defun emdwl-tool-test-inline ()
  (interactive)
  (emdwl-tool-open-inline "alacritty -e  bash -c 'echo 000 ;fish'" (cons (current-buffer) (point)))
  )

(defun emdwl-tool-add-advice ()
  (interactive)
  (advice-add 'scroll-up :after 'emdwl-tool-inline-scrool )
  (advice-add 'scroll-down :after 'emdwl-tool-inline-scrool )
  (add-hook 'window-state-change-functions 'emdwl-tool-client-window-change-update)
  )
(defun emdwl-tool-client-position-update ()
  "Update position and visible."
  (dolist (client emdwl-tool-client-list)
    (if (emdwl-tool-client-is-visible client)
        (if (equal (current-buffer) (car (cdr client)))
            (progn
              (emdwl-tool-resize-point (car client) (cdr client))
              ;;(emdwl-tool-client-visible (car client))
              )
          )
      (emdwl-tool-client-invisible (car client))
      )))
(defun emdwl-tool-client-window-change-update (&optional arg)
  "Update when window change."
  (if (or (emdwl-tool-is-quick)
          (not (minibufferp (current-buffer))))
      (dolist (client emdwl-tool-client-list)
        (progn
          (if (emdwl-tool-client-is-visible client)
              (progn
                (emdwl-tool-client-width-update (cdr client))
                ;; (setq buffer-c (current-buffer))
                ;; (pop-to-buffer  (car (cdr client)))
                ;; (set-buffer  (car (cdr client)))
                (emdwl-tool-resize-point (car client) (cdr client))
                ;; (pop-to-buffer buffer-c)
                (emdwl-tool-client-visible (car client))
                )
            (emdwl-tool-client-invisible (car client))
            )
          )
        )
    )
  )
(defun emdwl-tool-client-hs-change ()
  "Update when overlay invisible."
  (dolist (client emdwl-tool-client-list)
    (if (equal (current-buffer) (car (cdr client)))
        (if (emdwl-tool-is-visible-overlay (cdr client))
            (emdwl-tool-client-invisible (car client))
          (emdwl-tool-client-visible (car client))
          ))
    ))
(defun emdwl-tool-client-width-update (buffer-point)
  "Update client width."
  ;;(setq position (emdwl-tool-find-poistion-with-buffer-point buffer-point))
  (dolist (window-p (window-list))
    (if (equal (car buffer-point)
               (window-buffer window-p))
        (progn
          (setq emdwl-tool-client-width
                (- (window-pixel-width window-p) (window-font-width)))
          (if (not (null display-line-numbers-mode))
              (setq emdwl-tool-client-width
                    (* (- (window-text-width)
                          (line-number-display-width) 1)
                       (window-font-width)))
            )
          )
      )
    ))

(defun emdwl-tool-client-is-visible (client)
  "Check client visible.
CLIENT: (CLIENT-ID . (BUFFER . POINT))"
  (setq visible-flag 'nil)
  (dolist (window-p (window-list))
    (if (pos-visible-in-window-p (cdr (cdr client)) window-p)
        (if (and (equal (car (cdr client))
                        (window-buffer window-p))
                 (not (emdwl-tool-is-visible-overlay (cdr client)))
                 )
            (setq visible-flag 't)
          ))
    )
  (not (null visible-flag))
  )

(defun emdwl-tool-client-visible (client-id)
  (emdwl-newtags-client client-id 0)
  )
(defun emdwl-tool-client-invisible (client-id)
  (emdwl-newtags-client client-id 1)
  )
(defun emdwl-tool-client-close (client-id)
  "Client close and delete overlay."
  (dolist (client emdwl-tool-client-list)
    (if (equal client-id (car client))
        (progn
          (pop-to-buffer (car (cdr client)))
          (emdwl-close-client client-id)
          (emdwl-tool-delete-overlay (cdr (cdr client)))
          (delete client emdwl-tool-client-list)
          )
      )
    )
  )
(defun emdwl-tool-add-client-id-list (list-p)
  (dolist (client-id emdwl-tool-client-id-list)
    (if (equal (car client-id) (car list-p))
        (delete client-id emdwl-tool-client-id-list)
      )
    )
  (add-to-list 'emdwl-tool-client-id-list list-p)
  )
(defun emdwl-tool-add-client-id-list-update ()

  (setq  emdwl-tool-client-id-list 'nil)
  (emdwl-list)
  )
(defun emdwl-tool-is-quick ()
  "Deal with update is to quick."
  (setq time-tmp (current-time))
  (if (or (null emdwl-tool-time-list)
          (and
           (eq (nth 0 emdwl-tool-time-list) (nth 0 time-tmp))
           (eq (nth 1 emdwl-tool-time-list) (nth 1 time-tmp))
           (< (- (nth 2 emdwl-tool-time-list) (nth 2 time-tmp)) 300000)
           ))
      (null nil)
    (progn
      (setq emdwl-tool-time-list time-tmp)
      (null 't))
    )
  )
(defun emdwl-tool-choose-client ()
  (emdwl-tool-add-client-id-list-update)
  (setq tem-lis 'nil)
  (dolist (list-p emdwl-tool-client-id-list)
    (if (null (assoc (nth 0 list-p) emdwl-tool-client-list))
        (add-to-list 'tem-lis
                     (list (format "client-id-%S-%s-%s"
                                   (nth 0 list-p)
                                   (nth 1 list-p)
                                   (nth 2 list-p)
                                   (nth 3 list-p))
                           (nth 0 list-p))
                     ))
    )
  (nth 1 (assoc (completing-read
                 "Complete a foo: "
                 ;;'(("foobar1" 1) ("barfoo" 2) ("foobaz" 3) ("foobar2" 4))
                 tem-lis
                 nil t "client-id-")
                tem-lis
                ))
  )
(provide 'emdwl-tool)
;;; emdwl-tool.el ends here
