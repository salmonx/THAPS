#ifndef THAPS_SHARED_H
#define THAPS_SHARED_H

#define INI_EXISTS(n) INI_STR(n) != NULL

void dbug(THAPS_TS_DC const char* format, ...);
void dbug_start(THAPS_TS_D);
void dbug_stop(THAPS_TS_D);
char *thaps_itoa(int i);
char *substr(const char *pstr, int start, int numchars);
int strpos(char *haystack, char *needle);
int get_opline(zend_execute_data *current_execute_data);
zval *thaps_get_zval(THAPS_TS_DC THAPS_NODE *node, int node_type, zend_execute_data *execute_data);
void set_header(THAPS_TS_DC char *name, char *value);
char* get_current_time();

#endif

