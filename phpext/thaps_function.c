#include "php_thaps.h"
#include "thaps_shared.h"
#include "thaps_function.h"

extern ZEND_DECLARE_MODULE_GLOBALS(thaps);

int fcall_handler(ZEND_OPCODE_HANDLER_ARGS) {

	/* Only do this if THAPS is enabled */
	if (!THAPS_G(thaps_enabled))
		return ZEND_USER_OPCODE_DISPATCH;

	zend_op *opline;
	zend_class_entry *scope;
	zend_function *zfunction;
#if ZEND_MODULE_API_NO >= 20100525
	const char *filename;
	const char *class;
#else
    char *filename;
	char *class;
#endif
    char *fname;
	void **args;
	int arg_count;
	int function_id;
	int i;
	char* type;

	opline = execute_data->opline;
	filename = execute_data->op_array->filename;
	args = EG(argument_stack)->top;
	arg_count = opline->extended_value;

	if (opline->opcode == ZEND_DO_FCALL_BY_NAME) {

		fname = (char *)(execute_data->call->fbc->common.function_name);
        scope = execute_data->call->fbc->common.scope;

	} else {

#if ZEND_MODULE_API_NO >= 20100525
		fname = Z_STRVAL_P(opline->op1.zv);
#else
		fname = Z_STRVAL_P(&(opline)->op1.u.constant);
#endif
		scope = EG(current_execute_data)->function_state.function->common.scope;

	}

	class = (scope != NULL) ? scope->name : NULL;

	if ( strncmp(fname, "call_user_func_array", 20) == 0 || strncmp(fname, "call_user_func", 14) == 0 ) {

		zval *z = *((zval **) (args - arg_count));
		char *called_fname = z->value.str.val;
		int pos = strpos(called_fname, "::");

		if (pos >= 0) {
			/* Static method call (string as first argument: "class::method") */

			class = substr(called_fname, 0, pos);
			fname = substr(called_fname, pos + 2, strlen(called_fname));

		} else if (Z_TYPE_P(z) == IS_ARRAY) {

			zval **z_fname;
			zval **z_class;
			HashTable *table = Z_ARRVAL(*z);

			zend_hash_index_find(table, 0, (void**) &z_class);

			if (Z_TYPE_P(*z_class) == IS_OBJECT) {
				/* object method call */
				zend_class_entry *ce = Z_OBJCE_P(*z_class);
				class = ce->name;
			} else {
				/* static method call (array("class", "method") as first argument) */
				class = (*z_class)->value.str.val;
			}

			zend_hash_index_find(table, 1, (void**) &z_fname);
			fname = (*z_fname)->value.str.val;

		} else {
			fname = called_fname;
		}
	}

	/* Skip internal functions.. Find them here and return */
	if (zend_hash_find(EG(function_table), fname, strlen(fname)+1, (void**) &zfunction) == SUCCESS) {
		if (zfunction->type == ZEND_INTERNAL_FUNCTION) {
			dbug(THAPS_TS_CC "Skipping internal function call: %s", fname);
			return ZEND_USER_OPCODE_DISPATCH;
		}
	}

	dbug(THAPS_TS_CC "Function called: %s", fname);
	dbug(THAPS_TS_CC "  Class: %s", class);
	dbug(THAPS_TS_CC "  File: %s", filename);
	dbug(THAPS_TS_CC "  Line: %d", opline->lineno);

	char *c = thaps_itoa(THAPS_G(c_functions)++);

	bson_append_start_object( &THAPS_G(bson_functions), c );
	bson_append_string( &THAPS_G(bson_functions), "function", fname );
	bson_append_string( &THAPS_G(bson_functions), "file", filename );

	bson_append_int( &THAPS_G(bson_functions), "line", opline->lineno );
	if (class != NULL)
		bson_append_string( &THAPS_G(bson_functions), "class", class );
	bson_append_start_array( &THAPS_G(bson_functions), "arguments" );

	/* Determine all arguments for the function */
	for (i = 0; i < arg_count; i++) {
		zval *arg = *((zval **) (args - (arg_count - i)));
		char *type;

		switch (Z_TYPE_P(arg)) {
			case IS_LONG:			type = "long";		break;
			case IS_DOUBLE:			type = "double";	break;
			case IS_BOOL:			type = "boolean";	break;
			case IS_ARRAY:			type = "array";		break;
			case IS_OBJECT:			type = "object";	break;
			case IS_STRING:			type = "string";	break;
			case IS_RESOURCE:		type = "resource";	break;
			case IS_NULL: default:	type = NULL;		break;
		}

		dbug(THAPS_TS_CC "  Argument %d: %s", i, type);

		bson_append_start_object( &THAPS_G(bson_functions), thaps_itoa(i) );
		bson_append_int( &THAPS_G(bson_functions), "index", i );
		if (type != NULL)
			bson_append_string( &THAPS_G(bson_functions), "type", type );
		bson_append_finish_object( &THAPS_G(bson_functions) );

	}

	bson_append_finish_array( &THAPS_G(bson_functions) );
	bson_append_finish_object( &THAPS_G(bson_functions) );

	free(c);

	return ZEND_USER_OPCODE_DISPATCH;

}
