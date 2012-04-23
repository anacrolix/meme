(define (fib n)
  (cond ((= n 0) 0)
        ((= n 1) 1)
        (else (+ (fib (- n 1))
                 (fib (- n 2))))))

(define (fact-iter product counter max-count)
(if (> counter max-count)
  product
  (fact-iter (* counter product)
             (+ counter 1)
             max-count)))
(define (factorial n)
  (fact-iter 1 1 n))

