#include "ruby.h"
#include "ruby/encoding.h"
#define ICU_RUBY_ENCODING_INDEX(_idx) (_idx) = (rb_enc_to_index(rb_default_internal_encoding()) || rb_locale_encindex())

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

int icu_is_rb_enc_idx_as_utf_8(int enc_idx)
{
    return enc_idx == rb_ascii8bit_encindex() ||
        enc_idx == rb_usascii_encindex() ||
        enc_idx == rb_utf8_encindex();
}

int icu_is_rb_str_as_utf_8(VALUE str)
{
    return icu_is_rb_enc_idx_as_utf_8(ENCODING_GET(str));
}

int icu_rb_str_enc_idx(VALUE str)
{
    return ENCODING_GET(str);
}

