PHP_ARG_ENABLE(diseval, whether to enable diseval support,
[  --enable-diseval            Enable diseval support])

if test "$PHP_diseval" != "no"; then
  PHP_NEW_EXTENSION(diseval, diseval.c, $ext_shared)
fi

