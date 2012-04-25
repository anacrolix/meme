; double evaluations
(if (not (eq? 'else (car '(else))))
  (error))

; compound procedures unpack arg lists
;(define (a . a) a)
(define a (lambda a a))
(define b '(1 2 3))
(if (eq? b (apply a b)) #t (error))

(define c (lambda (a) (cdr a)))

