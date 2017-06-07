#include "icu.h"
#include "unicode/ucsdet.h"

#define GET_DETECTOR(_data) icu_detector_data* _data; \
                            TypedData_Get_Struct(self, icu_detector_data, &icu_detector_type, _data)

VALUE rb_cICU_CharsetDetector;
VALUE rb_cICU_CharsetDetector_Match;

typedef struct {
    VALUE rb_instance;
    UCharsetDetector* service;
    char* dummy_str; // used for reset
} icu_detector_data;

static void detector_free(void* _this)
{
    icu_detector_data* this = _this;
    if (this->dummy_str != NULL) {
        ruby_xfree(this->dummy_str);
    }
    ucsdet_close(this->service);
}

static size_t detector_memsize(const void* _)
{
    return sizeof(icu_detector_data);
}

static const rb_data_type_t icu_detector_type = {
    "icu/charset_detector",
    {NULL, detector_free, detector_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE detector_populate_match_struct(const UCharsetMatch* match)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t confidence = ucsdet_getConfidence(match, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    status = U_ZERO_ERROR;
    const char* name = ucsdet_getName(match, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    status = U_ZERO_ERROR;
    const char* language = ucsdet_getLanguage(match, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    return rb_struct_new(rb_cICU_CharsetDetector_Match,
                         rb_str_new_cstr(name),
                         INT2NUM(confidence),
                         rb_str_new_cstr(language));
}

VALUE detector_alloc(VALUE self)
{
    icu_detector_data* this;
    return TypedData_Make_Struct(self, icu_detector_data, &icu_detector_type, this);
}

VALUE detector_initialize(int argc, VALUE* argv, VALUE self)
{
    GET_DETECTOR(this);
    this->rb_instance = self;
    this->service = NULL;

    UErrorCode status = U_ZERO_ERROR;
    this->service = ucsdet_open(&status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    this->dummy_str = ALLOC_N(char, 1);
    this->dummy_str[0] = "\0";

    return self;
}

static void detector_reset_text(icu_detector_data* this)
{
    UErrorCode status = U_ZERO_ERROR;
    ucsdet_setText(this->service, this->dummy_str, 0, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
}

// rb_str must be a ruby String
static void detector_set_text(icu_detector_data* this, VALUE rb_str)
{
    UErrorCode status = U_ZERO_ERROR;
    ucsdet_setText(this->service, RSTRING_PTR(rb_str), RSTRING_LEN(rb_str), &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
}

//
// no charset appears to match the data.
// no input text has been provided
VALUE detector_detect(VALUE self, VALUE str)
{
    StringValue(str);
    GET_DETECTOR(this);

    detector_set_text(this, str);
    UErrorCode status = U_ZERO_ERROR;
    UCharsetMatch* match = ucsdet_detect(this->service, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    VALUE rb_match = detector_populate_match_struct(match);
    detector_reset_text(this);
    return rb_match;
}

VALUE detector_detect_all(VALUE self, VALUE str)
{
    StringValue(str);
    GET_DETECTOR(this);

    detector_set_text(this, str);

    UErrorCode status = U_ZERO_ERROR;
    int32_t len_matches = 0;
    UCharsetMatch** matches = ucsdet_detectAll(this->service, &len_matches, &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    VALUE result = rb_ary_new2(3); // pre-allocate some slots
    for (int32_t i = 0; i < len_matches; ++i) {
        rb_ary_push(result, detector_populate_match_struct(matches[i]));
    }
    detector_reset_text(this);
    return result;
}

VALUE detector_get_input_filter(VALUE self)
{
    GET_DETECTOR(this);
    return ucsdet_isInputFilterEnabled(this->service) != 0 ? Qtrue : Qfalse;
}

void detector_set_input_filter(VALUE self, VALUE flag)
{
    GET_DETECTOR(this);
    ucsdet_enableInputFilter(this->service, flag == Qtrue ? TRUE : FALSE);
}

VALUE detector_detectable_charsets(VALUE self)
{
    GET_DETECTOR(this);
    UErrorCode status = U_ZERO_ERROR;
    UEnumeration* charsets = ucsdet_getAllDetectableCharsets(this->service, &status);
    return icu_enum_to_rb_ary(charsets, status, 28);
}

void init_icu_charset_detector(void)
{
    rb_cICU_CharsetDetector = rb_define_class_under(rb_mICU, "CharsetDetector", rb_cObject);
    rb_define_alloc_func(rb_cICU_CharsetDetector, detector_alloc);
    rb_define_method(rb_cICU_CharsetDetector, "initialize", detector_initialize, -1);
    rb_define_method(rb_cICU_CharsetDetector, "detect", detector_detect, 1);
    rb_define_method(rb_cICU_CharsetDetector, "detect_all", detector_detect_all, 1);
    rb_define_method(rb_cICU_CharsetDetector, "input_filter", detector_get_input_filter, 0);
    rb_define_method(rb_cICU_CharsetDetector, "input_filter=", detector_set_input_filter, 1);
    rb_define_method(rb_cICU_CharsetDetector, "detectable_charsets", detector_detectable_charsets, 0);

    // define a Match struct in Ruby
    rb_cICU_CharsetDetector_Match = rb_struct_define_under(rb_cICU_CharsetDetector,
                                                           "Match",
                                                           "name",
                                                           "confidence",
                                                           "language",
                                                           NULL);
}

#undef GET_DETECTOR

/* vim: set expandtab sws=4 sw=4: */
