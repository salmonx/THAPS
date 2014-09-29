#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_thaps.h"
#include "thaps_shared.h"
#include "thaps_include.h"
#include "thaps_function.h"
#include "thaps_variables.h"

ZEND_DECLARE_MODULE_GLOBALS(thaps)

#if ZEND_MODULE_API_NO >= 20100525
static zend_function_entry thaps_functions[] = {
#else
static function_entry thaps_functions[] = {
#endif
    PHP_FE(thaps_enable, NULL)
    {NULL, NULL, NULL}
};

extern zend_module_entry thaps_module_entry;
#define phpext_thaps_ptr &thaps_module_entry

zend_module_entry thaps_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	THAPS_EXTNAME,
	thaps_functions,
	PHP_MINIT(thaps),
	PHP_MSHUTDOWN(thaps),
	PHP_RINIT(thaps),
	PHP_RSHUTDOWN(thaps),
	PHP_MINFO(thaps),
#if ZEND_MODULE_API_NO >= 20010901
	THAPS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_THAPS
ZEND_GET_MODULE(thaps)
#endif

static void php_thaps_init_globals(zend_thaps_globals *thaps_globals) {
	thaps_globals->thaps_enabled = 0;
	thaps_globals->request_uri = NULL;
    thaps_globals->debug_file = NULL;
}

PHP_INI_BEGIN()
PHP_INI_ENTRY("thaps.logfile", NULL, PHP_INI_ALL, NULL)
PHP_INI_ENTRY("thaps.dbhost", "127.0.0.1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("thaps.dbport", "27017", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("thaps.dbuser", "thaps", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("thaps.dbpass", "thaps", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("thaps.cookie", "THAPS", PHP_INI_ALL, NULL)
PHP_INI_END()

PHP_MINIT_FUNCTION(thaps) {
	ZEND_INIT_MODULE_GLOBALS(thaps, php_thaps_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	dbug_start(THAPS_TS_C);
    dbug(THAPS_TS_CC "THAPS PHP extension initialized (%s)", get_current_time());
    dbug(THAPS_TS_CC "PHP version ID: %i", PHP_VERSION_ID);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(thaps) {
	char buf[50];
	php_info_print_table_start();
	php_info_print_table_header(2, "THAPS Support", "enabled");
	php_info_print_table_row(2, "thaps.logfile", INI_STR("thaps.logfile"));
	php_info_print_table_row(2, "thaps.dbhost", INI_STR("thaps.dbhost"));
	php_info_print_table_row(2, "thaps.dbport", INI_STR("thaps.dbport"));
	php_info_print_table_row(2, "thaps.dbuser", INI_STR("thaps.dbuser"));
	php_info_print_table_row(2, "thaps.dbpass", INI_STR("thaps.dbpass"));
	snprintf(buf, sizeof(buf), "%s=ENABLED", INI_STR("thaps.cookie"));
	php_info_print_table_row(2, "thaps.cookie", buf);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}

PHP_RINIT_FUNCTION(thaps) {
    char *cookie;
    /* Retrieve the THAPS cookie variable */
    cookie = get_request_variable(THAPS_TS_CC "_COOKIE", INI_STR("thaps.cookie"));
	/* Lets add some hooks.. :-) */
	zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, include_or_eval_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL, fcall_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, fcall_handler);

    if (cookie != NULL && strncmp(cookie, "ENABLED", 7) == 0) {
		dbug(THAPS_TS_CC "THAPS cookie (%s) found - trace enabled", INI_STR("thaps.cookie"));
		init_thaps(THAPS_TS_C);
	}
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(thaps) {

	if (THAPS_G(thaps_enabled)) {

		bson_finish( &THAPS_G(bson_includes) );
		bson_finish( &THAPS_G(bson_variables) );
		bson_finish( &THAPS_G(bson_functions) );

		dbug(THAPS_TS_CC "Total number of includes: %d", THAPS_G(c_includes));
		dbug(THAPS_TS_CC "Total number of function calls: %d", THAPS_G(c_functions));

		dbug(THAPS_TS_CC "Request shutdown: Saving data to database");

		if (THAPS_G(request_uri) != NULL)
			bson_append_string( &THAPS_G(bson_final), "request", THAPS_G(request_uri) );
		if (THAPS_G(request_file) != NULL)
			bson_append_string( &THAPS_G(bson_final), "filename", THAPS_G(request_file) );
		bson_append_bson( &THAPS_G(bson_final), "includes", &THAPS_G(bson_includes) );
		bson_append_bson( &THAPS_G(bson_final), "variables", &THAPS_G(bson_variables) );
		bson_append_bson( &THAPS_G(bson_final), "functions", &THAPS_G(bson_functions) );
		bson_finish( &THAPS_G(bson_final) );

		db_insert(THAPS_TS_CC "local.thaps", &THAPS_G(bson_final));

		bson_destroy( &THAPS_G(bson_includes) );
		bson_destroy( &THAPS_G(bson_variables) );
		bson_destroy( &THAPS_G(bson_functions) );
		bson_destroy( &THAPS_G(bson_final) );

		db_close(THAPS_TS_C);
    }
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(thaps) {
	UNREGISTER_INI_ENTRIES();
    dbug(THAPS_TS_CC "THAPS module shutdown");
	dbug_stop(THAPS_TS_C);
	return SUCCESS;
}

PHP_FUNCTION(thaps_enable) {
	dbug(THAPS_TS_CC "thaps_enable() called - trace enabled");
	if (!THAPS_G(thaps_enabled))
		init_thaps(THAPS_TS_C);
}

void init_thaps(THAPS_TS_D) {

	bson_oid_t b_oid;
	char request_id[25];

	THAPS_G(request_uri) = sapi_getenv("REQUEST_URI", sizeof("REQUEST_URI")-1 TSRMLS_CC);
	THAPS_G(request_file) = sapi_getenv("SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME")-1 TSRMLS_CC);

	THAPS_G(thaps_enabled) = 1;
	THAPS_G(c_includes) = 0;
	THAPS_G(c_variables) = 0;
	THAPS_G(c_functions) = 0;

	db_open(THAPS_TS_C);

	/* Generate a new oid and append it to the bson to build */
	bson_oid_gen(&b_oid);
	bson_oid_to_string(&b_oid, request_id);

	bson_init( &THAPS_G(bson_final) );
	bson_append_oid(&THAPS_G(bson_final), "_id", &b_oid);

	dbug(THAPS_TS_CC "Generated oid: %s", request_id);
	set_header(THAPS_TS_CC "X-THAPS-RequestId", request_id);

	bson_init( &THAPS_G(bson_includes) );
	bson_init( &THAPS_G(bson_variables) );
	bson_init( &THAPS_G(bson_functions) );

	fetch_all_request_variables(THAPS_TS_CC "_GET");
	fetch_all_request_variables(THAPS_TS_CC "_POST");
	fetch_all_request_variables(THAPS_TS_CC "_REQUEST");
	fetch_all_request_variables(THAPS_TS_CC "_COOKIE");
}

void db_open(THAPS_TS_D) {

	if (INI_EXISTS("thaps.dbhost")) {

		char *host = INI_STR("thaps.dbhost");
		int port = INI_INT("thaps.dbport");

		dbug(THAPS_TS_CC "Opening database connection: %s:%d", host, port);

		if (mongo_connect(&THAPS_G(db), host, port) != MONGO_OK) {
			dbug(THAPS_TS_CC "Unable to connect to database");
			exit(1);
		} else if (mongo_cmd_authenticate(&THAPS_G(db), "local", INI_STR("thaps.dbuser"), INI_STR("thaps.dbpass")) != MONGO_OK) {
			dbug(THAPS_TS_CC "Unable to authenticate.");
			dbug(THAPS_TS_CC "  Username: %s", INI_STR("thaps.dbuser"));
			dbug(THAPS_TS_CC "  Password: %s", INI_STR("thaps.dbpass"));
			exit(1);
		} else {
			dbug(THAPS_TS_CC "Successfully connected to database");
		}
	} else {
		dbug(THAPS_TS_CC "No dbhost defined in conf. Database connection disabled");
	}
}

void db_insert(THAPS_TS_DC char *coll, bson *b) {
	if (INI_EXISTS("thaps.dbhost")) {
		dbug(THAPS_TS_CC "Inserting document into %s", coll);
		mongo_insert(&THAPS_G(db), coll, b, NULL);
	}
}

void db_close(THAPS_TS_D) {
	if (INI_EXISTS("thaps.dbhost")) {
		dbug(THAPS_TS_CC "Closing database connection");
		mongo_disconnect(&THAPS_G(db));
		mongo_destroy(&THAPS_G(db));
	}
}
