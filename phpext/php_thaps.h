#ifndef PHP_THAPS_H
#define PHP_THAPS_H

#define THAPS_VERSION "1.0"
#define THAPS_EXTNAME "THAPS"
#define THAPS_AUTHORS "Torben Jensen, Heine Pedersen"
#define THAPS_COPYRIGHT "Copyright (c) 2012"

#ifdef ZTS
#include "TSRM.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"
#include <stdio.h>
#include <sys/file.h>
#include <time.h>
#include <bson.h>
#include <mongo.h>

ZEND_BEGIN_MODULE_GLOBALS(thaps)
	zend_bool thaps_enabled;
	char *request_uri;
	char *request_file;
	FILE *debug_file;
	bson bson_final;
	bson bson_includes;
	bson bson_variables;
	bson bson_functions;
	int c_includes;
	int c_variables;
	int c_functions;
	mongo db;
ZEND_END_MODULE_GLOBALS(thaps)

#if ZEND_MODULE_API_NO >= 20100525
#define THAPS_NODE znode_op
#else
#define THAPS_NODE znode
#endif

#ifdef ZTS
#define THAPS_G(v) TSRMG(thaps_globals_id, zend_thaps_globals *, v)
#define THAPS_TS_D TSRMLS_D
#define THAPS_TS_C TSRMLS_C
#define THAPS_TS_DC TSRMLS_D, 
#define THAPS_TS_CC TSRMLS_C, 
#else
#define THAPS_G(v) (thaps_globals.v)
#define THAPS_TS_D
#define THAPS_TS_C
#define THAPS_TS_DC
#define THAPS_TS_CC
#endif

PHP_MINIT_FUNCTION(thaps);
PHP_MSHUTDOWN_FUNCTION(thaps);
PHP_RINIT_FUNCTION(thaps);
PHP_RSHUTDOWN_FUNCTION(thaps);
PHP_MINFO_FUNCTION(thaps);
PHP_FUNCTION(thaps_enable);
void init_thaps(THAPS_TS_D);
void db_close(THAPS_TS_D);
void db_open(THAPS_TS_D);
void db_insert(THAPS_TS_DC char *coll, bson *b);

#endif
