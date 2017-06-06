#include "icu.h"
#include "ruby/encoding.h"

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
