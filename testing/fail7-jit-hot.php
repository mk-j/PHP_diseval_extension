<?php
// The eval() opline lives in a hot function that the JIT compiles (function JIT compiles it
// on load, tracing JIT after the warm-up). The eval branch is only taken at the very end.
function f(int $x, bool $do_eval) {
    if ($do_eval) {
        return eval('return "eval ran";');
    }
    return $x + 1;
}
$s = 0;
for ($i = 0; $i < 20000; $i++) { $s = f($s, false); }
echo "warm $s\n";
echo f(0, true), "\n";
