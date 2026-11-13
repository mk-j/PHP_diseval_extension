<?php
// Ordinary code (functions, closures, recursion, include) is unaffected.
function fib($n) { return $n < 2 ? $n : fib($n - 1) + fib($n - 2); }
$f = fn($x) => $x * 2;
echo fib(20) + $f(1), "\n";
