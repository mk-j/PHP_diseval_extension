# PHP diseval extension

The purpose of the diseval php extension is to disable eval in php.
While suhosin can disable eval in older versions of php.  Diseval 
is able to disable eval in both php5 and php7 with version 0.1 and php8 in 0.2.

When this php extension is enabled, the following will yield PHP Errors:

```php
eval("echo 44;");
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : eval()'d code on line 1
```php
$newfunc = create_function('$a,$b', 'return $a+$b;');
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : runtime-created function on line 1

```php
echo mb_ereg_replace("[0-3]", '4*4', "4434","e" );
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : mbregex replace on line 1
```php
assert('is_array(15)');
```

PHP Fatal error:  DISEVAL - Use of eval is forbidden in /path/to/script.php(4) : assert code on line 1
