#include "icu.h"
#include "unicode/uspoof.h"

#define GET_SPOOF_CHECKER(_data) icu_spoof_checker_data* _data; \
                                 TypedData_Get_Struct(self, icu_spoof_checker_data, &icu_spoof_checker_type, _data)

VALUE rb_cICU_SpoofChecker;
VALUE rb_mChecks;
VALUE rb_mRestrictionLevel;

typedef struct {
    VALUE rb_instance;
    USpoofChecker* service;
} icu_spoof_checker_data;

static void spoof_checker_free(void* _this)
{
    icu_spoof_checker_data* this = _this;
    uspoof_close(this->service);
}

static size_t spoof_checker_memsize(const void* _)
{
    return sizeof(icu_spoof_checker_data);
}

static const rb_data_type_t icu_spoof_checker_type = {
    "icu/spoof_checker",
    {NULL, spoof_checker_free, spoof_checker_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE spoof_checker_alloc(VALUE self)
{
    icu_spoof_checker_data* this;
    return TypedData_Make_Struct(self, icu_spoof_checker_data, &icu_spoof_checker_type, this);
}

VALUE spoof_checker_initialize(VALUE self)
{
    GET_SPOOF_CHECKER(this);
    this->rb_instance = self;
    this->service = FALSE;

    UErrorCode status = U_ZERO_ERROR;
    this->service = uspoof_open(&status);
    if (U_FAILURE(status)) {
        icu_rb_raise_icu_error(status);
    }

    return self;
}

static inline VALUE spoof_checker_get_restriction_level_internal(const icu_spoof_checker_data* this)
{
    URestrictionLevel level = uspoof_getRestrictionLevel(this->service);
    return INT2NUM(level);
}

VALUE spoof_checker_get_restriction_level(VALUE self)
{
    GET_SPOOF_CHECKER(this);
    return spoof_checker_get_restriction_level_internal(this);
}

VALUE spoof_checker_set_restriction_level(VALUE self, VALUE level)
{
    GET_SPOOF_CHECKER(this);
    uspoof_setRestrictionLevel(this->service, NUM2INT(level));
    return spoof_checker_get_restriction_level_internal(this);
}

static inline VALUE spoof_checker_get_checks_internal(const icu_spoof_checker_data* this)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t checks = uspoof_getChecks(this->service, &status);
    if (U_FAILURE(status)) {
        icu_rb_raise_icu_error(status);
    }
    return INT2NUM(checks);
}

VALUE spoof_checker_get_checks(VALUE self)
{
    GET_SPOOF_CHECKER(this);
    return spoof_checker_get_checks_internal(this);
}

VALUE spoof_checker_set_checks(VALUE self, VALUE checks)
{
    GET_SPOOF_CHECKER(this);

    UErrorCode status = U_ZERO_ERROR;
    uspoof_setChecks(this->service, NUM2INT(checks), &status);
    if (U_FAILURE(status)) {
        icu_rb_raise_icu_error(status);
    }
    return spoof_checker_get_checks_internal(this);
}

VALUE spoof_checker_confusable(VALUE self, VALUE str_a, VALUE str_b)
{
    StringValue(str_a);
    StringValue(str_b);
    GET_SPOOF_CHECKER(this);

    VALUE tmp_a = icu_ustring_from_rb_str(str_a);
    VALUE tmp_b = icu_ustring_from_rb_str(str_b);
    UErrorCode status = U_ZERO_ERROR;
    int32_t result = uspoof_areConfusable(this->service,
                                          icu_ustring_ptr(tmp_a),
                                          icu_ustring_len(tmp_a),
                                          icu_ustring_ptr(tmp_b),
                                          icu_ustring_len(tmp_b),
                                          &status);

    return INT2NUM(result);
}

VALUE spoof_checker_get_skeleton(VALUE self, VALUE str)
{
    StringValue(str);
    GET_SPOOF_CHECKER(this);

    VALUE in = icu_ustring_from_rb_str(str);
    VALUE out = icu_ustring_init_with_capa_enc(icu_ustring_capa(in), ICU_RUBY_ENCODING_INDEX);
    int retried = FALSE;
    int32_t len_bytes;
    UErrorCode status = U_ZERO_ERROR;
    do {
        // UTF-8 version does the conversion internally so we relies on UChar version here!
        len_bytes = uspoof_getSkeleton(this->service, 0 /* deprecated */,
                                       icu_ustring_ptr(in), icu_ustring_len(in),
                                       icu_ustring_ptr(out), icu_ustring_capa(out),
                                       &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(out, len_bytes + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);

    return icu_ustring_to_rb_enc_str_with_len(out, len_bytes);
}

VALUE spoof_checker_check(VALUE self, VALUE rb_str)
{
    StringValue(rb_str);
    GET_SPOOF_CHECKER(this);

    UErrorCode status = U_ZERO_ERROR;
    int32_t result = 0;

    // TODO: Migrate to uspoof_check2UTF8 once it's not draft
    if (icu_is_rb_str_as_utf_8(rb_str)) {
       result = uspoof_checkUTF8(this->service,
                                 RSTRING_PTR(rb_str),
                                 RSTRING_LENINT(rb_str),
                                 NULL,
                                 &status);
    } else {
        VALUE in = icu_ustring_from_rb_str(rb_str);
        // TODO: Migrate to uspoof_check once it's not draft
        result = uspoof_check(this->service,
                              icu_ustring_ptr(in),
                              icu_ustring_len(in),
                              NULL,
                              &status);
    }
    if (U_FAILURE(status)) {
        icu_rb_raise_icu_error(status);
    }

    return INT2NUM(result);
}

static const char* k_checks_name = "@checks";

VALUE spoof_checker_available_checks(VALUE klass)
{
    VALUE iv = rb_iv_get(klass, k_checks_name);
    if (NIL_P(iv)) {
        iv = rb_hash_new();
        rb_hash_aset(iv, ID2SYM(rb_intern("single_script_confusable")), INT2NUM(USPOOF_SINGLE_SCRIPT_CONFUSABLE));
        rb_hash_aset(iv, ID2SYM(rb_intern("mixed_script_confusable")), INT2NUM(USPOOF_MIXED_SCRIPT_CONFUSABLE));
        rb_hash_aset(iv, ID2SYM(rb_intern("whole_script_confusable")), INT2NUM(USPOOF_WHOLE_SCRIPT_CONFUSABLE));
        rb_hash_aset(iv, ID2SYM(rb_intern("confusable")), INT2NUM(USPOOF_CONFUSABLE));
        // USPOOF_ANY_CASE deprecated in 58
        rb_hash_aset(iv, ID2SYM(rb_intern("restriction_level")), INT2NUM(USPOOF_RESTRICTION_LEVEL));
        // USPOOF_SINGLE_SCRIPT deprecated in 51
        rb_hash_aset(iv, ID2SYM(rb_intern("invisible")), INT2NUM(USPOOF_INVISIBLE));
        rb_hash_aset(iv, ID2SYM(rb_intern("char_limit")), INT2NUM(USPOOF_CHAR_LIMIT));
        rb_hash_aset(iv, ID2SYM(rb_intern("mixed_numbers")), INT2NUM(USPOOF_MIXED_NUMBERS));
        rb_hash_aset(iv, ID2SYM(rb_intern("all_checks")), INT2NUM(USPOOF_ALL_CHECKS));
        rb_hash_aset(iv, ID2SYM(rb_intern("aux_info")), INT2NUM(USPOOF_AUX_INFO));
        rb_iv_set(klass, k_checks_name, iv);
    }
    return iv;
}

static const char* k_restriction_level_name = "@restriction_levels";

VALUE spoof_checker_available_restriction_levels(VALUE klass)
{
    VALUE iv = rb_iv_get(klass, k_restriction_level_name);
    if (NIL_P(iv)) {
        iv = rb_hash_new();
        rb_hash_aset(iv, ID2SYM(rb_intern("ascii")), INT2NUM(USPOOF_ASCII));
        rb_hash_aset(iv, ID2SYM(rb_intern("single_script_restrictive")), INT2NUM(USPOOF_SINGLE_SCRIPT_RESTRICTIVE));
        rb_hash_aset(iv, ID2SYM(rb_intern("highly_restrictive")), INT2NUM(USPOOF_HIGHLY_RESTRICTIVE));
        rb_hash_aset(iv, ID2SYM(rb_intern("moderately_restrictive")), INT2NUM(USPOOF_MODERATELY_RESTRICTIVE));
        rb_hash_aset(iv, ID2SYM(rb_intern("minimally_restrictive")), INT2NUM(USPOOF_MINIMALLY_RESTRICTIVE));
        rb_hash_aset(iv, ID2SYM(rb_intern("unrestrictive")), INT2NUM(USPOOF_UNRESTRICTIVE));
        rb_hash_aset(iv, ID2SYM(rb_intern("restriction_level_mask")), INT2NUM(USPOOF_RESTRICTION_LEVEL_MASK));
        rb_hash_aset(iv, ID2SYM(rb_intern("undefined_restrictive")), INT2NUM(USPOOF_UNDEFINED_RESTRICTIVE));
        rb_iv_set(klass, k_restriction_level_name, iv);
    }
    return iv;
}

void init_icu_spoof_checker(void)
{
    rb_cICU_SpoofChecker = rb_define_class_under(rb_mICU, "SpoofChecker", rb_cObject);
    rb_define_singleton_method(rb_cICU_SpoofChecker, "available_checks", spoof_checker_available_checks, 0);
    rb_define_singleton_method(rb_cICU_SpoofChecker, "available_restriction_levels", spoof_checker_available_restriction_levels, 0);
    rb_define_alloc_func(rb_cICU_SpoofChecker, spoof_checker_alloc);
    rb_define_method(rb_cICU_SpoofChecker, "initialize", spoof_checker_initialize, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level", spoof_checker_get_restriction_level, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level=", spoof_checker_set_restriction_level, 1);
    rb_define_method(rb_cICU_SpoofChecker, "check", spoof_checker_check, 1);
    rb_define_method(rb_cICU_SpoofChecker, "checks", spoof_checker_get_checks, 0);
    rb_define_method(rb_cICU_SpoofChecker, "checks=", spoof_checker_set_checks, 1);
    rb_define_method(rb_cICU_SpoofChecker, "confusable?", spoof_checker_confusable, 2);
    rb_define_method(rb_cICU_SpoofChecker, "get_skeleton", spoof_checker_get_skeleton, 1);
}

#undef DEFINE_SPOOF_ENUM_CONST
#undef GET_SPOOF_CHECKER

/* vim: set expandtab sws=4 sw=4: */
