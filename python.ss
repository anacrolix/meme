(define (all . iter)
  (if (null? iter) #t (if (car iter) (apply all (cdr iter)) #f)))

(define (any . iter)
  (if (null? iter) #f (if (car iter) #t (apply any (cdr iter)))))

(define (combinations p r)
  (cond ((or (null? p) (= 0 r)) '())
        ((= 1 r) (map (lambda (i) (list i)) p))
        (else (append
                (map (lambda (c) (cons (car p) c))
                     (combinations (cdr p) (- r 1)))
                (combinations (cdr p) r)))))

(combinations '(a b c) 2)
