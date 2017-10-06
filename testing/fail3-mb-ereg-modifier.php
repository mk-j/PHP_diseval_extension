<?php

/*
php -d display_errors=On -dextension=modules/diseval.so fail3-mb-ereg-modifier.php;

Should say something like:
PHP Fatal error: DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : runtime-created function on line 1
*/

echo mb_ereg_replace("[0-3]", '4*4', "4434","e" );


