<?php

/*
php -d display_errors=On -dextension=modules/diseval.so fail1-eval.php;

Should say something like:
PHP Fatal error: DISEVAL - Use of eval is forbidden by configuration in /path/to/script.php(10) : eval()'d code on line 1
*/

eval("echo 's';");


