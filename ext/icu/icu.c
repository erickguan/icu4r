#include "ruby.h"

VALUE rb_mICU;

void Init_icu(void) {
    rb_mICU = rb_define_module("ICU");
}

/* vim: set noet sws=4 sw=4: */
