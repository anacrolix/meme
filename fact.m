(= > (^ a b (if (< b a) #t #f)))
(= -- (^ a (- a 1)))
(= fact (^ n 
  (if (> n 1) (* n (fact (-- n))) 1)))

(fact 4)
