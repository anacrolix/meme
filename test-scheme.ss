; double evaluations
(if (not (eq? 'else (car '(else))))
  (error))

; compound procedures unpack arg lists
;(define (a . a) a)
(define a (lambda a a))
(define b '(1 2 3))
(if (eq? b (apply a b)) #t (error))

(define c (lambda (a) (cdr a)))

; begin
(if (not (= 3 (begin 1 2 3)))
  (error))
(if (not (= 6 ((lambda a (apply begin a)) 2 4 6))) (error))

; define formals
(define (d . e) e)
(if (/= b (apply d b)) (error))
(if (/= b (apply (lambda a a) b)) (error))

; cond
(define (test-cond) 
  (cond ((eq? a 2) 1)
        ((= 4 7) 2)
        (else 3)))
(define! a 2)
(if (/= 1 (test-cond)) (error))
(set! a 4)
(if (/= 3 (test-cond)) (error))
(undef a)
