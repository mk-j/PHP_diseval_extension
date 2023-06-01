/* diseval extension for PHP */

#ifndef PHP_DISEVAL_H
# define PHP_DISEVAL_H

extern zend_module_entry diseval_module_entry;
# define phpext_diseval_ptr &diseval_module_entry

# define PHP_DISEVAL_VERSION "0.2.0"

# if defined(ZTS) && defined(COMPILE_DL_DISEVAL)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

void diseval_execute_ex(zend_execute_data *execute_data);

#endif	/* PHP_DISEVAL_H */
