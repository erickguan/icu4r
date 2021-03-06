#include "icu.h"

VALUE rb_mICU;

void Init_icu(void)
{
    rb_mICU = rb_define_module("ICU");
    init_internal_encoding();
    init_rb_errors();
    init_icu_collator();
    init_icu_normalizer();
    init_icu_spoof_checker();
    init_icu_transliterator();
    init_icu_charset_detector();
    init_icu_locale();
}

/* vim: set expandtab sws=4 sw=4: */
