(unpack '(else 3))
(unpack '((eq? a 2) 2) '((= 4 7) 3 4) '(else 3))
(define a 2)
(cond ((eq? a 2) 1)
      ((= 4 7) 2)
      (else 3))