/* diseval extension for PHP */

#ifndef PHP_DISEVAL_H
# define PHP_DISEVAL_H

extern zend_module_entry diseval_module_entry;
# define phpext_diseval_ptr &diseval_module_entry

# define PHP_DISEVAL_VERSION "0.3.0"

# if defined(ZTS) && defined(COMPILE_DL_DISEVAL)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

ZEND_BEGIN_MODULE_GLOBALS(diseval)
	bool enabled;
ZEND_END_MODULE_GLOBALS(diseval)

# define DISEVAL_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(diseval, v)

#endif	/* PHP_DISEVAL_H */
