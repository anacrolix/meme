(define (> a b) (< b a))
(define (-- a) (- a 1))
(define (factorial n)
  (if (> n 1)
    (* n (factorial (-- n)))
    1))

(factorial 7)
