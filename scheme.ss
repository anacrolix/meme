(define (not x) (if x #f #t))
(define (/= . a) (not (apply = a)))
(define (> a b) (< b a))
(define (caar x) (car (car x)))
(define (cdar x) (cdr (car x)))
(define (append x y)
  (if (null? x) y (cons (car x) (append (cdr x) y))))
(define (cond . clauses)
  (if (eq? 'else (caar clauses))
    (append '(begin) (cdar clauses))
    (apply cond (list (cdr clauses)))))



