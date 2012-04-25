; double evaluations
(if (not (eq? 'else (car '(else))))
  (error))

; compound procedures unpack arg lists
(define (a . a) a)
(define b '(1 2 3))
(if (eq? b (apply a b)) #t (error))
