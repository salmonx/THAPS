#include "php_thaps.h"
#include "thaps_shared.h"
#include "thaps_include.h"

extern ZEND_DECLARE_MODULE_GLOBALS(thaps);

int include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS) {

	/* Only do this if THAPS is enabled */
	if (!THAPS_G(thaps_enabled))
		return ZEND_USER_OPCODE_DISPATCH;

	zend_file_handle file_handle;
	zend_op *opline = execute_data->opline;
	zval *inc_file = NULL;

	// Do not support eval()'s
#if ZEND_MODULE_API_NO >= 20100525
	if (opline->op2.constant == ZEND_EVAL) {
#else
	if (opline->op2.u.constant.value.lval == ZEND_EVAL) {
#endif

		dbug(THAPS_TS_CC "eval() detected. I'm not touching that!");

	} else {

		// We have an include/require request
		inc_file = thaps_get_zval(THAPS_TS_CC &opline->op1, opline->op1_type, execute_data);
		
        // Check if we can access the file
		if (zend_stream_open(inc_file->value.str.val, &file_handle TSRMLS_CC) == SUCCESS) {

			dbug(THAPS_TS_CC "File inclusion detected");
			dbug(THAPS_TS_CC "  File: %s", execute_data->op_array->filename);
			dbug(THAPS_TS_CC "  On line: %d", get_opline(execute_data));
			dbug(THAPS_TS_CC "  Includes file: %s", file_handle.opened_path);

			char *c = thaps_itoa(THAPS_G(c_includes)++);

			bson_append_start_object( &THAPS_G(bson_includes), c );
			bson_append_string( &THAPS_G(bson_includes), "current", execute_data->op_array->filename );
			bson_append_int( &THAPS_G(bson_includes), "line", get_opline(execute_data) );
			bson_append_string( &THAPS_G(bson_includes), "included", file_handle.opened_path );
			bson_append_finish_object( &THAPS_G(bson_includes) );

			free(c);

			// Don't be dirty, lets clean up
			zend_file_handle_dtor(&file_handle TSRMLS_CC);

		} else {

			dbug(THAPS_TS_CC "Could not include file: %s", inc_file->value.str.val);
			dbug(THAPS_TS_CC "  File: %s", execute_data->op_array->filename);
			dbug(THAPS_TS_CC "  Line: %d", opline->lineno);
		}
	}
    return ZEND_USER_OPCODE_DISPATCH;
}
