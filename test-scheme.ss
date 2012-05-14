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
; begin is a special, this doesn't have to make sense
;(if (not (= 6 ((lambda a (apply begin a)) 2 4 6))) (error))

; define formals
(define (d . e) e)
(if (/= b (apply d b)) (error))
(if (/= b (apply (lambda a a) b)) (error))

; cond
(define (test-cond) 
  (cond ((eq? a 2) 1)
        ((= 4 7) 2)
        (else 3)))
;(define! a 2)
(set! a 2)
(if (/= 1 (test-cond)) (error))
(set! a 4)
(if (= 3 (test-cond)) (error))
;(undef a)

; map
(if (/= '(2 12 30) (map (lambda (x y) (* x y)) '(1 3 5) '(2 4 6))) (error))

; combinations
(if (/= '((a b) (a c) (b c)) (combinations '(a b c) 2)) (error))

; try
(if (/= 6 
        (let ((x 2) (y 3))
          (* x y)))
  (error))

(if (/= 35
        (let ((x 2) (y 3))
          (let ((x 7)
                (z (+ x y)))
            (* z x))))
  (error))
