<?php
// A script must not be able to switch the protection off (diseval.enabled is PHP_INI_SYSTEM).
var_dump(ini_set('diseval.enabled', '0'));
eval("echo 'eval ran';");
