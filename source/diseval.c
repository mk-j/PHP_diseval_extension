/* diseval extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_diseval.h"
#include "diseval_arginfo.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

void (*zend_execute_old)(zend_execute_data *execute_data);

/* {{{ void diseval_info() */
PHP_FUNCTION(diseval_info)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "diseval");
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(diseval)
{
#if defined(ZTS) && defined(COMPILE_DL_DISEVAL)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

PHP_MINIT_FUNCTION(diseval)
{
	zend_execute_old = zend_execute_ex;
	zend_execute_ex = diseval_execute_ex;
	return SUCCESS;
}

PHP_MINFO_FUNCTION(diseval)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Diseval support", "enabled");
	php_info_print_table_end();
}

PHP_MSHUTDOWN_FUNCTION(diseval)
{
	zend_execute_ex = zend_execute_old;
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(diseval)
{
	return SUCCESS;
}

void diseval_execute_ex(zend_execute_data *execute_data)
{
#if PHP_VERSION_ID >= 70000
	const zend_op_array *op_array = &execute_data->func->op_array;
#else
	const zend_op_array *op_array = execute_data->op_array;
#endif
	if (op_array->type == ZEND_EVAL_CODE) {
		zend_error(E_ERROR, "DISEVAL - Use of eval is forbidden");
		zend_bailout();
	}
	zend_execute_old(execute_data);
}

/* {{{ diseval_module_entry */
zend_module_entry diseval_module_entry = {
	STANDARD_MODULE_HEADER,
	"diseval",					/* Extension name */
	ext_functions,				/* zend_function_entry */
	PHP_MINIT(diseval),			/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(diseval),		/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(diseval),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(diseval),		/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(diseval),			/* PHP_MINFO - Module info */
	PHP_DISEVAL_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DISEVAL
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(diseval)
#endif
