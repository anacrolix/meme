(define (not x) (if x #f #t))
(define (/= . a) (not (apply = a)))
(define (> a b) (< b a))
(define (caar x) (car (car x)))
(define (cdar x) (cdr (car x)))
(define (append x y)
  (if (null? x) y (cons (car x) (append (cdr x) y))))
;(define cond (macro (lambda clauses
;  (define (unpack first . rest)
;    (if (eq? 'else (car first))
;      (append '(begin) (cdr first))
;      (list 'if (car first) (cons 'begin (cdr first)) (unpack rest))))
;  (unpack clauses))))

