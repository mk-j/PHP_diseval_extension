#ifndef PHP_DISEVAL_H
#define PHP_DISEVAL_H

#include "php.h"

extern zend_module_entry diseval_module_entry;
#define phpext_diseval_ptr &diseval_module_entry

#ifdef PHP_WIN32
#define PHP_DISEVAL_API __declspec(dllexport)
#else
#define PHP_DISEVAL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif


PHP_MINIT_FUNCTION(diseval);
PHP_MSHUTDOWN_FUNCTION(diseval);
PHP_RINIT_FUNCTION(diseval);
PHP_RSHUTDOWN_FUNCTION(diseval);
PHP_MINFO_FUNCTION(diseval);

PHP_FUNCTION(diseval_info);

void diseval_execute_ex(zend_execute_data *execute_data TSRMLS_DC);


#endif

