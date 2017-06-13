#include "icu.h"

static rb_encoding* ascii_enc;
static rb_encoding* utf8_enc;
static ID ID_to_s;

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

VALUE rb_str_enc_to_ascii_as_utf8(VALUE str)
{
    str = rb_funcall(str, ID_to_s, 0);
    // sanitize, will raise Encoding::UndefinedConversionError if anything wrong
    str = rb_str_encode(str, rb_enc_from_encoding(ascii_enc), 0, Qnil);
    rb_enc_associate(str, utf8_enc);
    return str;
}

void init_internal_encoding(void)
{
    ascii_enc = rb_ascii8bit_encoding();
    utf8_enc = rb_utf8_encoding();
    ID_to_s = rb_intern("to_s");
}