;;;; -*- Mode: lisp; indent-tabs-mode: nil -*-

;;; Copyright (C) 2015, Dmitry Ignatiev <lovesan.ru at gmail.com>

;;; Permission is hereby granted, free of charge, to any person
;;; obtaining a copy of this software and associated documentation
;;; files (the "Software"), to deal in the Software without
;;; restriction, including without limitation the rights to use, copy,
;;; modify, merge, publish, distribute, sublicense, and/or sell copies
;;; of the Software, and to permit persons to whom the Software is
;;; furnished to do so, subject to the following conditions:

;;; The above copyright notice and this permission notice shall be
;;; included in all copies or substantial portions of the Software.

;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;;; NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;;; DEALINGS IN THE SOFTWARE.

(in-package #:cl-libmonitor)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (define-foreign-library libmonitor
    (t (:default "libmonitor")))
  
  (use-foreign-library libmonitor))

(defcfun (%lm-create "lm_create" :library libmonitor)
    :pointer)

(defcfun (%lm-free "lm_free" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-enter "lm_enter" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-try-enter "lm_try_enter" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-exit "lm_exit" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-wait "lm_wait" :library libmonitor)
    :int
  (m :pointer)
  (ms :int))

(defcfun (%lm-pulse "lm_pulse" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-pulse-all "lm_pulse_all" :library libmonitor)
    :int
  (m :pointer))

(defcfun (%lm-is-valid "lm_is_valid" :library libmonitor)
    :int
  (m :pointer))

(defstruct (lm-monitor-core
            (:constructor %lm-mc (pointer))
            (:conc-name %lm-mc-))
  (pointer (null-pointer) :type foreign-pointer))

(defstruct (lm-monitor
            (:predicate lm-monitor-p)
            (:constructor %lm-monitor (core))
            (:conc-name %lm-monitor-))
  (core (%lm-mc (null-pointer)) :type lm-monitor-core))

(declaim (inline %lm-pointer))
(defun %lm-pointer (monitor)
  (%lm-mc-pointer (%lm-monitor-core monitor)))

(define-condition libmonitor-error (error)
  ((%monitor :accessor lm-error-monitor
             :initform nil
             :initarg :monitor)))

(define-condition libmonitor-failure (libmonitor-error)
  ())

(define-condition libmonitor-timeout (libmonitor-error)
  ((%time :accessor lm-error-time
          :initarg :time
          :initform -1)))

(define-condition libmonitor-invalid-pointer (libmonitor-error)
  ((%pointer :accessor lm-error-pointer
             :initarg :pointer
             :initform (null-pointer))))

(defmacro check-error (monitor code &optional (timeout 0))
  (let ((c (gensym))
        (m (gensym))
        (tm (gensym))
        (p (gensym)))
    `(let* ((,c (the fixnum ,code))
            (,m ,monitor)
            (,p (and monitor (%lm-pointer ,m)))
            (,tm (the fixnum ,timeout)))
       (case ,c
         (0 (values))
         (2 (error 'libmonitor-timeout :monitor ,m :time ,tm))
         (3 (error 'libmonitor-invalid-pointer :monitor ,m :pointer ,p))
         (t (error 'libmonitor-failure :monitor ,m)))
       (values))))

(defun lm-monitor ()
  (let ((p (%lm-create)))
    (when (null-pointer-p p)
      (error 'libmonitor-failure))
    (let* ((core (%lm-mc p))
           (mon (%lm-monitor core)))
      (finalize mon (lambda ()
                      (unless (null-pointer-p (%lm-mc-pointer core))
                        (%lm-free p)
                        (setf (%lm-mc-pointer core) (null-pointer)))))
      mon)))

(defun lm-valid-p (monitor)
  (declare (type lm-monitor monitor))
  (zerop (%lm-is-valid (%lm-pointer monitor))))

(defun lm-free (monitor)
  (declare (type lm-monitor monitor))
  (let ((p (%lm-pointer monitor)))
    (unless (null-pointer-p p)
      (check-error monitor (%lm-free p))
      (setf (%lm-mc-pointer (%lm-monitor-core monitor))
            (null-pointer)))
    (values)))

(defun lm-enter (monitor)
  (declare (type lm-monitor monitor))
  (let ((p (%lm-pointer monitor)))
    (check-error monitor (%lm-enter p))))

(defun lm-try-enter (monitor)
  (declare (type lm-monitor monitor))
  (let* ((p (%lm-pointer monitor))
         (r (the fixnum (%lm-try-enter p))))
    (case r
      (0 t)
      (1 nil)
      (t (check-error monitor r)))))

(defun lm-exit (monitor)
  (declare (type lm-monitor monitor))
  (let ((p (%lm-pointer monitor)))
    (check-error monitor (%lm-exit p))))

(defmacro with-lm-monitor ((var monitor) &body body)
  `(let ((,var ,monitor))
     (declare (type lm-monitor ,var))
     (unwind-protect (progn ,@body) (lm-free ,var))))

(defmacro with-lm-lock ((var monitor) &body body)
  `(let ((,var ,monitor))
     (declare (type lm-monitor ,var))
     (lm-enter ,var)
     (unwind-protect (progn ,@body) (lm-exit ,var))))

(defun lm-wait (monitor &optional timeout)
  (declare (type lm-monitor monitor)
           (type (or null (integer 0 #.most-positive-fixnum)) timeout))
  (let* ((timeout (or timeout -1))
         (p (%lm-pointer monitor))
         (r (the fixnum (%lm-wait p timeout))))
    (case r
      (0 t)
      (2 nil)
      (t (check-error monitor r timeout)))))

(defun lm-pulse (monitor)
  (declare (type lm-monitor monitor))
  (let ((p (%lm-pointer monitor)))
    (check-error monitor (%lm-pulse p))))

(defun lm-pulse-all (monitor)
  (declare (type lm-monitor monitor))
  (let ((p (%lm-pointer monitor)))
    (check-error monitor (%lm-pulse-all p))))

(defmethod print-object ((monitor lm-monitor) stream)
  (print-unreadable-object (monitor stream :type t :identity t)
    (format stream "~s ~s"
            :valid (lm-valid-p monitor)))
  monitor)


;; vim: ft=lisp et
