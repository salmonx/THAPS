
PHP_ARG_ENABLE(thaps, whether to enable thaps support, [ --enable-thaps   Enable THAPS support])
if test "$PHP_THAPS" = "yes"; then
  AC_DEFINE(HAVE_THAPS, 1, [Whether you have THAPS])
  PHP_NEW_EXTENSION(thaps, thaps.c thaps_shared.c thaps_include.c thaps_function.c thaps_variables.c, $ext_shared)
  CFLAGS="-lmongoc -DMONGO_HAVE_STDINT $CFLAGS"
fi
