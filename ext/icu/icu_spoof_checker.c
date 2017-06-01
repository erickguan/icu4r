#include "icu.h"
#include "unicode/uspoof.h"

#define GET_SPOOF_CHECKER(_data) icu_spoof_checker_data* _data; \
                                 TypedData_Get_Struct(self, icu_spoof_checker_data, &icu_spoof_checker_type, _data)

VALUE rb_cICU_SpoofChecker;
VALUE rb_mChecks;
VALUE rb_mRestrictionLevel;
static ID ID_single_script_confusable;
static ID ID_nfkc;
static ID ID_nfkc_cf;
static ID ID_compose;
static ID ID_decompose;

typedef struct {
    VALUE instance;
    USpoofChecker* checker;
} icu_spoof_checker_data;

static void spoof_checker_free(void* _this)
{
    icu_spoof_checker_data* this = _this;
    uspoof_close(this->checker);
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
    this->instance = self;
    this->checker = FALSE;

    UErrorCode status = U_ZERO_ERROR;
    this->checker = uspoof_open(&status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    return self;
}

VALUE spoof_checker_get_restriction_level(VALUE self)
{
    GET_SPOOF_CHECKER(this);
    URestrictionLevel level = uspoof_getRestrictionLevel(this->checker);

    return INT2NUM(level);
}

void spoof_checker_set_restriction_level(VALUE self, VALUE level)
{
    GET_SPOOF_CHECKER(this);

    uspoof_setRestrictionLevel(this->checker, NUM2INT(level));
}

VALUE spoof_checker_get_checks(VALUE self)
{
    GET_SPOOF_CHECKER(this);

    UErrorCode status = U_ZERO_ERROR;
    int32_t check = uspoof_getChecks(this->checker, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    return INT2NUM(check);
}

void spoof_checker_set_checks(VALUE self, VALUE checks)
{
    GET_SPOOF_CHECKER(this);

    UErrorCode status = U_ZERO_ERROR;
    uspoof_setChecks(this->checker, NUM2INT(checks), &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
}

VALUE spoof_checker_confusable(VALUE self, VALUE str_a, VALUE str_b)
{
    StringValue(str_a);
    StringValue(str_b);
    GET_SPOOF_CHECKER(this);

    VALUE tmp_a = icu_uchar_string_alloc(rb_cICU_UCharString);
    VALUE tmp_b = icu_uchar_string_alloc(rb_cICU_UCharString);
    icu_uchar_string_replace(tmp_a, str_a);
    icu_uchar_string_replace(tmp_b, str_b);
    UErrorCode status = U_ZERO_ERROR;
    int32_t result = uspoof_areConfusable(this->checker,
                                          icu_uchar_string_ptr(tmp_a),
                                          icu_uchar_string_len(tmp_a),
                                          icu_uchar_string_ptr(tmp_b),
                                          icu_uchar_string_len(tmp_b),
                                          &status);

    return INT2NUM(result);
}

VALUE spoof_checker_get_skeleton(VALUE self, VALUE str)
{
    GET_SPOOF_CHECKER(this);
    StringValue(str);

    UErrorCode status = U_ZERO_ERROR;
    if (icu_is_rb_str_as_utf_8(str)) {
        int32_t len = RSTRING_LEN(str);
        int32_t dest_capa = len * 2 + 1;
        char* dest = ALLOC_N(char, dest_capa);
        int retry = FALSE;
        int32_t len_bytes;
        do {
            len_bytes = uspoof_getSkeletonUTF8(this->checker,
                                               0,
                                               StringValuePtr(str),
                                               RSTRING_LEN(str),
                                               dest,
                                               dest_capa,
                                               &status);
            if (!retry && status == U_BUFFER_OVERFLOW_ERROR) {
                retry = TRUE;
                dest_capa = len_bytes;
                REALLOC_N(dest, char, dest_capa);
                status = U_ZERO_ERROR;
            } else if (U_FAILURE(status)) {
                rb_raise(rb_eICU_Error, u_errorName(status));
            } else { // retry == true && U_SUCCESS(status)
                break;
            }
        } while (retry);

        return rb_str_new_cstr(dest);
    } else {
        VALUE in = icu_uchar_string_alloc(rb_cICU_UCharString);
        in = icu_uchar_string_replace(in, str);
        VALUE out = icu_uchar_string_alloc(rb_cICU_UCharString);
        icu_uchar_string_new_capa_enc(out, RSTRING_LEN(str) * 2 + 1, ICU_RUBY_ENCODING_INDEX);
        int retry = FALSE;
        int32_t len_bytes;

        UErrorCode status = U_ZERO_ERROR;
        do {
           len_bytes = uspoof_getSkeleton(this->checker,
                                          0,
                                          in,
                                          RSTRING_LEN(str),
                                          icu_uchar_string_ptr(out),
                                          icu_uchar_string_capa(out),
                                          &status);
           if (!retry && status == U_BUFFER_OVERFLOW_ERROR) {
               retry = TRUE;
               icu_uchar_string_set_capa(out, len_bytes);
               status = U_ZERO_ERROR;
           } else if (U_FAILURE(status)) {
               rb_raise(rb_eICU_Error, u_errorName(status));
           } else { // retry == true && U_SUCCESS(status)
               break;
           }
        } while (retry);

        return icu_uchar_string_to_rb_enc_str(out);
    }
}

#define DEFINE_SPOOF_ENUM_CONST(_MODULE, _NAME) rb_define_const(_MODULE, #_NAME, INT2NUM(USPOOF_##_NAME))

void init_icu_spoof_checker(void)
{
    rb_cICU_SpoofChecker = rb_define_class_under(rb_mICU, "SpoofChecker", rb_cObject);
    rb_mChecks = rb_define_module_under(rb_cICU_SpoofChecker, "Checks");
    rb_mRestrictionLevel = rb_define_module_under(rb_cICU_SpoofChecker, "RestrictionLevel");
    rb_include_module(rb_cICU_SpoofChecker, rb_mChecks);
    rb_include_module(rb_cICU_SpoofChecker, rb_mRestrictionLevel);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, SINGLE_SCRIPT_CONFUSABLE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, MIXED_SCRIPT_CONFUSABLE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, WHOLE_SCRIPT_CONFUSABLE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, CONFUSABLE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, ANY_CASE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, RESTRICTION_LEVEL);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, SINGLE_SCRIPT);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, INVISIBLE);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, CHAR_LIMIT);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, MIXED_NUMBERS);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, ALL_CHECKS);
    DEFINE_SPOOF_ENUM_CONST(rb_mChecks, AUX_INFO);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, ASCII);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, SINGLE_SCRIPT_RESTRICTIVE);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, HIGHLY_RESTRICTIVE);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, MODERATELY_RESTRICTIVE);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, MINIMALLY_RESTRICTIVE);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, UNRESTRICTIVE);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, RESTRICTION_LEVEL_MASK);
    DEFINE_SPOOF_ENUM_CONST(rb_mRestrictionLevel, UNDEFINED_RESTRICTIVE);

    rb_define_alloc_func(rb_cICU_SpoofChecker, spoof_checker_alloc);
    rb_define_method(rb_cICU_SpoofChecker, "initialize", spoof_checker_initialize, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level", spoof_checker_get_restriction_level, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level=", spoof_checker_set_restriction_level, 1);
    rb_define_method(rb_cICU_SpoofChecker, "checks", spoof_checker_get_checks, 0);
    rb_define_method(rb_cICU_SpoofChecker, "checks=", spoof_checker_set_checks, 1);
    rb_define_method(rb_cICU_SpoofChecker, "confusable?", spoof_checker_confusable, 2);
    rb_define_method(rb_cICU_SpoofChecker, "get_skeleton", spoof_checker_get_skeleton, 1);
}

#undef DEFINE_SPOOF_ENUM_CONST
#undef GET_SPOOF_CHECKER

/* vim: set expandtab sws=4 sw=4: */
