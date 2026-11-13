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

#if PHP_VERSION_ID < 80000
# error "diseval >= 0.3 requires PHP 8.0+, use diseval 0.1 for PHP 5/7"
#endif

#if PHP_VERSION_ID >= 80200
# define DISEVAL_COMPILE_STRING_ARGS zend_string *source_string, const char *filename, zend_compile_position position
# define DISEVAL_COMPILE_STRING_CALL source_string, filename, position
#else
# define DISEVAL_COMPILE_STRING_ARGS zend_string *source_string, const char *filename
# define DISEVAL_COMPILE_STRING_CALL source_string, filename
#endif

/* The previous compiler hook, so other extensions' hooks keep working (chaining). */
static zend_op_array *(*diseval_orig_compile_string)(DISEVAL_COMPILE_STRING_ARGS);
static zend_op_array *diseval_compile_string(DISEVAL_COMPILE_STRING_ARGS);

ZEND_DECLARE_MODULE_GLOBALS(diseval)

/* diseval.enabled is PHP_INI_SYSTEM on purpose: it can be set in php.ini or in
 * the server/pool config (e.g. php_admin_value), but a script can NOT turn the
 * protection off with ini_set() or .user.ini. */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("diseval.enabled", "1", PHP_INI_SYSTEM, OnUpdateBool, enabled, zend_diseval_globals, diseval_globals)
PHP_INI_END()

static PHP_GINIT_FUNCTION(diseval)
{
#if defined(COMPILE_DL_DISEVAL) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	diseval_globals->enabled = 1;
}

/* {{{ void diseval_info() */
PHP_FUNCTION(diseval_info)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "diseval");
}
/* }}} */


PHP_MINIT_FUNCTION(diseval)
{
	REGISTER_INI_ENTRIES();
	diseval_orig_compile_string = zend_compile_string;
	zend_compile_string = diseval_compile_string;
	return SUCCESS;
}

PHP_MINFO_FUNCTION(diseval)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Diseval support", DISEVAL_G(enabled) ? "enabled" : "disabled (diseval.enabled=0)");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

PHP_MSHUTDOWN_FUNCTION(diseval)
{
	zend_compile_string = diseval_orig_compile_string;
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* Is the engine compiling the argument of a userland `eval(...)` right now?
 *
 * zend_compile_string() is also used by C code through zend_eval_string():
 * `php -r`, `php -a`, phpdbg and debuggers such as Xdebug (watches, conditional
 * breakpoints, "evaluate expression"). Those must keep working, so we only block
 * when the currently executing userland opline is the ZEND_INCLUDE_OR_EVAL
 * opcode in its ZEND_EVAL mode, i.e. a literal `eval()` in PHP code. */
static zend_always_inline bool diseval_is_userland_eval(void)
{
	zend_execute_data *ex = EG(current_execute_data);

	if (!ex || !ex->func || !ZEND_USER_CODE(ex->func->type) || !ex->opline) {
		return 0;
	}
	return ex->opline->opcode == ZEND_INCLUDE_OR_EVAL
		&& ex->opline->extended_value == ZEND_EVAL;
}

static zend_op_array *diseval_compile_string(DISEVAL_COMPILE_STRING_ARGS)
{
	if (DISEVAL_G(enabled) && diseval_is_userland_eval()) {
		/* E_ERROR bails out: the code is never compiled nor executed. */
		zend_error_noreturn(E_ERROR, "DISEVAL - Use of eval is forbidden");
	}
	return diseval_orig_compile_string(DISEVAL_COMPILE_STRING_CALL);
}

/* {{{ diseval_module_entry */
zend_module_entry diseval_module_entry = {
	STANDARD_MODULE_HEADER,
	"diseval",					/* Extension name */
	ext_functions,				/* zend_function_entry */
	PHP_MINIT(diseval),			/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(diseval),		/* PHP_MSHUTDOWN - Module shutdown */
	NULL,						/* PHP_RINIT - Request initialization */
	NULL,						/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(diseval),			/* PHP_MINFO - Module info */
	PHP_DISEVAL_VERSION,		/* Version */
	PHP_MODULE_GLOBALS(diseval),	/* Module globals */
	PHP_GINIT(diseval),			/* GINIT */
	NULL,						/* GSHUTDOWN */
	NULL,						/* RPOSTSHUTDOWN */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_DISEVAL
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(diseval)
#endif
