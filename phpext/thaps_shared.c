#include "php_thaps.h"
#include "thaps_shared.h"

#if ZEND_MODULE_API_NO >= 20121212
#define THAPS_T(offset) (*EX_TMP_VAR(execute_data, offset))
#define THAPS_CV(i) (*EX_CV_NUM(execute_data, i))
#else
#define THAPS_T(offset) (*(temp_variable *)((char*)execute_data->Ts + offset))
#define THAPS_CV(i) execute_data->CVs[i]
#endif

extern ZEND_DECLARE_MODULE_GLOBALS(thaps);

void dbug(THAPS_TS_DC const char* format, ...) {

    if (THAPS_G(debug_file) == NULL) {
        printf("WARNING: Debug disabled. Reason: file '%s' not opened. Permission problem?\n", INI_STR("thaps.logfile"));
        return;
    }

	if (INI_EXISTS("thaps.logfile")) {
		va_list args;
		va_start(args, format);

		if (flock(fileno(THAPS_G(debug_file)), LOCK_EX) == -1) {
			printf("Unable to lock file\n");
		} else {
			vfprintf(THAPS_G(debug_file), format, args);
			fprintf(THAPS_G(debug_file), "\n");
			fflush(THAPS_G(debug_file));
			flock(fileno(THAPS_G(debug_file)), LOCK_UN);
		}
		va_end(args);
	}
}

void dbug_start(THAPS_TS_D) {
	if (THAPS_G(debug_file) == NULL && INI_EXISTS("thaps.logfile")) {
		THAPS_G(debug_file) = fopen(INI_STR("thaps.logfile"), "w");
	}
}

void dbug_stop(THAPS_TS_D) {
	if (THAPS_G(debug_file) != NULL && INI_EXISTS("thaps.logfile")) {
		fclose(THAPS_G(debug_file));
		THAPS_G(debug_file) = NULL;
	}
}

char *thaps_itoa(int i) {
	int len;
	char *buf;

	len = snprintf(NULL, 0, "%d", i);
	buf = malloc(sizeof(char)*(len+1));
	snprintf(buf, sizeof(buf), "%d", i);
	buf[len] = '\0';
	return buf;
}

char *substr(const char *pstr, int start, int numchars) {
	char *pnew = malloc(numchars + 1);
	strncpy(pnew, pstr + start, numchars);
	pnew[numchars] = '\0';
	return pnew;
}

int strpos(char *haystack, char *needle) {
   char *p = strstr(haystack, needle);
   return p ? (p - haystack) : -1;
}

int get_opline(zend_execute_data *current_execute_data) {
	int lineno = 0;
	if (current_execute_data->opline) {
		lineno = current_execute_data->opline->lineno;
	} else if (current_execute_data->prev_execute_data && current_execute_data->prev_execute_data->opline) {
		lineno = current_execute_data->prev_execute_data->opline->lineno;
	}
	return lineno;
}

zval *thaps_get_zval(THAPS_TS_DC THAPS_NODE *node, int node_type, zend_execute_data *execute_data) {

	switch (node_type) {
		case IS_CONST:
#if ZEND_MODULE_API_NO >= 20100525
            return node->zv;
#else
			return &(node->u.constant);
#endif
		case IS_VAR:
#if ZEND_MODULE_API_NO >= 20100525
            return THAPS_T(node->var).var.ptr;
#else
            return THAPS_T(node->u.var).var.ptr;
#endif
		case IS_TMP_VAR:
#if ZEND_MODULE_API_NO >= 20100525
            return &THAPS_T(node->var).tmp_var;
#else
			return &THAPS_T(node->u.var).tmp_var;
#endif
		case IS_CV: {
#if ZEND_MODULE_API_NO >= 20100525
            zval ***ret = &THAPS_CV(node->var);
#else
			zval ***ret = &THAPS_CV(node->u.var);
#endif
			if (!*ret) {
#if ZEND_MODULE_API_NO >= 20100525
                zend_compiled_variable *cv = &EG(active_op_array)->vars[node->var];
#else
				zend_compiled_variable *cv = &EG(active_op_array)->vars[node->u.var];
#endif
				if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void**)ret)==FAILURE) {
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					return &EG(uninitialized_zval);
				}
			}
			return **ret;
		}

		case IS_UNUSED:
		default:
			return NULL;
	}
}

void set_header(THAPS_TS_DC char *name, char *value) {
	int str_size = strlen(name) + strlen(value) + 3;
	char header[str_size];
	snprintf(header, str_size, "%s: %s", name, value);

    sapi_header_line ctr = {0};
    ctr.line = header;
    ctr.line_len = strlen(header);
    sapi_header_op(SAPI_HEADER_ADD, &ctr TSRMLS_CC);
}

char* get_current_time() {
	time_t current_time;
	struct tm *local_time;
	int buf_len = 9;
	time(&current_time);
	local_time = localtime(&current_time);

	char *buf = malloc(sizeof(char *) * buf_len);
	strftime(buf, buf_len, "%H:%M:%S", local_time);
	buf[8] = '\0';

	return buf;
}
