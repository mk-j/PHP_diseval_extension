<?php

/*
php -d display_errors=On -dextension=modules/diseval.so fail2-create-function.php;

Should say something like:
PHP Fatal error: DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : runtime-created function on line 1
*/

$newfunc = create_function('$a,$b', 'return $a+$b;');
$r = $newfunc(2,4);
var_dump($r);


