#include "icu.h"
#include "unicode/ucol.h"

#define GET_COLLATOR(_data) icu_collator_data* _data; \
                            TypedData_Get_Struct(self, icu_collator_data, &icu_collator_type, _data)

VALUE rb_cICU_Collator;
static ID ID_actual;

typedef struct {
    VALUE collator_instance;
    int enc_idx; // TODO: reexamine the necessary for this?
    UCollator* collator;
} icu_collator_data;

static void collator_free(void* _this)
{
    icu_collator_data* this = _this;
    ucol_close(this->collator);
}

static size_t collator_memsize(const void* _)
{
    return sizeof(icu_collator_data);
}

static const rb_data_type_t icu_collator_type = {
    "icu/collator",
    {NULL, collator_free, collator_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE collator_alloc(VALUE self)
{
    icu_collator_data* this;

    return TypedData_Make_Struct(self, icu_collator_data, &icu_collator_type, this);
}

VALUE collator_initialize(VALUE self, VALUE locale)
{
    GET_COLLATOR(this);

    this->enc_idx = 0;
    this->collator_instance = self;
    UErrorCode status = U_ZERO_ERROR;
    this->collator = ucol_open(StringValueCStr(locale), &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    return self;
}

/*ULOC_ACTUAL_LOCALE
  This is locale the data actually comes from.

  ULOC_VALID_LOCALE
  This is the most specific locale supported by ICU. */
VALUE collator_locale(int argc, VALUE* argv, VALUE self)
{
    GET_COLLATOR(this);
    VALUE actual;

    rb_scan_args(argc, argv, "01", &actual);
    if (NIL_P(actual)) {
        actual = ID2SYM(ID_actual);
    }

    ULocDataLocaleType type = ULOC_ACTUAL_LOCALE;
    if (SYM2ID(actual) != ID_actual) {
        type = ULOC_VALID_LOCALE;
    }
    UErrorCode status = U_ZERO_ERROR;
    const char* locale_str = ucol_getLocaleByType(this->collator, type, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    return locale_str != NULL ? rb_str_new_cstr(locale_str) : Qnil;
}

VALUE collator_compare(VALUE self, VALUE str_a, VALUE str_b)
{
    StringValue(str_a);
    StringValue(str_b);
    GET_COLLATOR(this);
    UCollationResult result = UCOL_EQUAL;

    if (icu_is_rb_str_as_utf_8(str_a) &&
        icu_is_rb_str_as_utf_8(str_b)) {
        UErrorCode status = U_ZERO_ERROR;
        result = ucol_strcollUTF8(this->collator,
                                  StringValuePtr(str_a),
                                  RSTRING_LEN(str_a),
                                  StringValuePtr(str_b),
                                  RSTRING_LEN(str_b),
                                  &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    } else {
        VALUE tmp_a = icu_uchar_string_alloc(rb_cICU_UCharString);
        VALUE tmp_b = icu_uchar_string_alloc(rb_cICU_UCharString);
        icu_uchar_string_replace(tmp_a, str_a);
        icu_uchar_string_replace(tmp_b, str_b);
        result = ucol_strcoll(this->collator,
                              icu_uchar_string_ptr(tmp_a),
                              icu_uchar_string_len(tmp_a),
                              icu_uchar_string_ptr(tmp_b),
                              icu_uchar_string_len(tmp_b));
    }

    return INT2FIX(result);
}

void init_icu_collator(void)
{
    ID_actual = rb_intern("actual");

    rb_cICU_Collator = rb_define_class_under(rb_mICU, "Collator", rb_cObject);
    rb_define_alloc_func(rb_cICU_Collator, collator_alloc);
    rb_define_method(rb_cICU_Collator, "initialize", collator_initialize, 1);
    rb_define_method(rb_cICU_Collator, "locale", collator_locale, -1);
    rb_define_method(rb_cICU_Collator, "compare", collator_compare, 2);
}

#undef GET_COLLATOR

/* vim: set expandtab sws=4 sw=4: */
