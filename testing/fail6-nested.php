<?php
// eval() reached through functions, methods, closures and call_user_func is still blocked.
class A { function run($c) { return (function () use ($c) { return eval($c); })(); } }
function go($c) { return call_user_func([new A, 'run'], $c); }
for ($i = 0; $i < 2000; $i++) { $x = strlen(str_repeat('x', $i)); } // warm up JIT
echo go("return 'eval ran';");
