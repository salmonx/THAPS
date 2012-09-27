#include "php_thaps.h"
#include "thaps_shared.h"

extern ZEND_DECLARE_MODULE_GLOBALS(thaps);

void fetch_all_request_variables(THAPS_TS_DC char *var) {

    zval **tmpvar;
    char *tmpstr;
    HashTable *table;

    if (zend_hash_find(&EG(symbol_table), var, strlen(var)+1, (void**)&tmpvar) != FAILURE) {
        table = Z_ARRVAL(**tmpvar);
        while (zend_hash_get_current_data(table, (void**)&tmpvar) == SUCCESS) {

            zend_hash_get_current_key(table, &tmpstr, NULL, 0);
            dbug(THAPS_TS_CC "User input: $%s[\"%s\"] = %s", var, tmpstr, (*tmpvar)->value.str.val);

			char *c = thaps_itoa(THAPS_G(c_variables)++);

			bson_append_start_object( &THAPS_G(bson_variables), c );
			bson_append_string( &THAPS_G(bson_variables), "var", var );
			bson_append_string( &THAPS_G(bson_variables), "name", tmpstr );
			bson_append_string( &THAPS_G(bson_variables), "value", (*tmpvar)->value.str.val );
			bson_append_finish_object( &THAPS_G(bson_variables) );

			free(c);

            zend_hash_move_forward(table);
        }
        zend_hash_internal_pointer_reset(table);
    }
}

char* get_request_variable(THAPS_TS_DC char *var, char *name) {

    zval **tmpvar;
    char *tmpstr;
    HashTable *table;

    if (zend_hash_find(&EG(symbol_table), var, strlen(var)+1, (void**)&tmpvar) != FAILURE) {

        table = Z_ARRVAL(**tmpvar);

        while (zend_hash_get_current_data(table, (void**)&tmpvar) == SUCCESS) {

            zend_hash_get_current_key(table, &tmpstr, NULL, 0);

            if (strncmp(tmpstr, name, strlen(name)) == 0)
				return (*tmpvar)->value.str.val;

            zend_hash_move_forward(table);
        }
        zend_hash_internal_pointer_reset(table);
    }
    return NULL;
}


