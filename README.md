# PHP_DISEVAL_Extension

The purpose of the diseval php extension is to disable eval in php.
Previously suhosin took care of this for php5, but suhosin7 is not
yet stable for php7, so we are led to use the diseval extension.

When this php extension is enabled, the following will yield PHP Errors:

```
eval("echo 44;");
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden by configuration in /path/to/script.php(4) : eval()'d code on line 1
```
$newfunc = create_function('$a,$b', 'return $a+$b;');
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden by configuration in /path/to/script.php(4) : runtime-created function on line 1

```
echo mb_ereg_replace("[0-3]", '4*4', "4434","e" );
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden by configuration in /path/to/script.php(4) : mbregex replace on line 1
