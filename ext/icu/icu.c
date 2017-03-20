#include "icu.h"

VALUE rb_mICU;

void Init_icu(void)
{
    rb_mICU = rb_define_module("ICU");
    init_icu_errors();
    init_icu_collator();
    init_icu_normalizer();
}

/* vim: set expandtab sws=4 sw=4: */
