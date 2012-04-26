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

