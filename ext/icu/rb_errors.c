#include "icu.h"

VALUE rb_eICU_Error;
VALUE rb_eICU_InvalidParameterError;

void init_rb_errors(void)
{
    rb_eICU_Error = rb_define_class_under(rb_mICU, "Error", rb_eStandardError);
    rb_define_alias(rb_eICU_Error, "error", "message");
    rb_eICU_InvalidParameterError = rb_define_class_under(rb_mICU, "InvalidParameterError", rb_eStandardError);
    rb_define_alias(rb_eICU_InvalidParameterError, "error", "message");
}

/* vim: set expandtab sws=4 sw=4: */
