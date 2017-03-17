#include "icu.h"

int icu_choose_encoding(void)
{
    int ruby_encoding;
    ICU_RUBY_ENCODING_INDEX(ruby_encoding);

    int utf8 = rb_utf8_encindex();
    if (ruby_encoding == rb_ascii8bit_encindex() ||
        ruby_encoding == rb_usascii_encindex() ||
        ruby_encoding == utf8) {
        return utf8;
    }

    if (ruby_encoding == rb_enc_find_index("UTF-16BE")) {
        return ruby_encoding;
    } else {
        return rb_enc_find_index("UTF-16LE");
    }
}

VALUE rb_mICU;

void Init_icu(void)
{
    rb_mICU = rb_define_module("ICU");
    init_icu_errors();
    init_icu_collator();
}

/* vim: set expandtab sws=4 sw=4: */
