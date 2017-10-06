<?php

/*
php -d display_errors=On -dextension=modules/diseval.so fail4-assert.php;

Should say something like:
PHP Fatal error: DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : runtime-created function on line 1
*/
if (PHP_MAJOR_VERSION>=7 && ini_get('zend.assertions')==1)
{
    echo assert('is_string(4);');
}
else if (PHP_MAJOR_VERSION>=5)
{
    echo assert('is_string(4);');
}
else
{
    echo "FAILED: unable to test php diseval for assertions, check php.ini\n";
    echo "zend.assertions setting, should be set to 1 in cli/non-production mode\n";
}
