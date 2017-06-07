#include "icu.h"
#include "unicode/utrans.h"

#define GET_TRANSLITERATOR(_data) icu_transliterator_data* _data; \
                                  TypedData_Get_Struct(self, icu_transliterator_data, &icu_transliterator_type, _data)

VALUE rb_cICU_Transliterator;
static ID ID_forward;
static ID ID_reverse;

typedef struct {
    VALUE rb_instance;
    UTransliterator* service;
} icu_transliterator_data;

static void transliterator_free(void* _this)
{
    icu_transliterator_data* this = _this;
    utrans_close(this->service);
}

static size_t transliterator_memsize(const void* _)
{
    return sizeof(icu_transliterator_data);
}

static const rb_data_type_t icu_transliterator_type = {
    "icu/transliterator",
    {NULL, transliterator_free, transliterator_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE transliterator_alloc(VALUE self)
{
    icu_transliterator_data* this;
    return TypedData_Make_Struct(self, icu_transliterator_data, &icu_transliterator_type, this);
}

VALUE transliterator_initialize(int argc, VALUE* argv, VALUE self)
{
    GET_TRANSLITERATOR(this);
    this->rb_instance = self;
    this->service = NULL;

    VALUE id;
    VALUE direction;
    VALUE rules;
    rb_scan_args(argc, argv, "12", &id, &rules, &direction);
    StringValue(id);
    if (!NIL_P(rules)) {
        StringValue(rules);
    }
    if (NIL_P(direction)) {
        direction = ID2SYM(ID_forward);
    } else {
        // TODO: handle invalid direction
    }

    VALUE u_id = icu_ustring_from_rb_str(id);
    UTransDirection u_direction = UTRANS_REVERSE;
    if (SYM2ID(direction) == ID_forward) {
        u_direction = UTRANS_FORWARD;
    }
    VALUE u_rules;
    if (!NIL_P(rules)) {
        u_rules = icu_ustring_from_rb_str(rules);
    }
    UParseError parser_error;
    UErrorCode status = U_ZERO_ERROR;
    this->service = utrans_openU(icu_ustring_ptr(u_id),
                                        icu_ustring_len(u_id),
                                        u_direction,
                                        NIL_P(rules) ? NULL : icu_ustring_ptr(u_rules),
                                        NIL_P(rules) ? 0 : icu_ustring_len(u_rules),
                                        &parser_error, // TODO: should be possible to interpolate
                                        &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }
    if (this->service == NULL) {
        rb_raise(rb_eICU_Error, "Transliterator can't be created.");
    }

    return self;
}

VALUE transliterator_transliterate(VALUE self, VALUE str)
{
    StringValue(str);
    GET_TRANSLITERATOR(this);

    VALUE u_str = icu_ustring_from_rb_str(str);
    UErrorCode status = U_ZERO_ERROR;
    int32_t original_len = icu_ustring_len(u_str);
    int32_t capa = icu_ustring_capa(u_str);
    int32_t len;
    int32_t limit;
    int retried = FALSE;
    do {
        len = limit = original_len;

        utrans_transUChars(this->service,
                           icu_ustring_ptr(u_str), &len, capa,
                           0 /* always start from the beginning */, &limit,
                           &status);

        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            u_str = icu_ustring_from_rb_str(str);
            capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            icu_ustring_resize(u_str, capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);

    return icu_ustring_to_rb_enc_str_with_len(u_str, len);
}

VALUE transliterator_unicode_id(VALUE self)
{
    GET_TRANSLITERATOR(this);

    int32_t result_len = 0;
    UChar* result = utrans_getUnicodeID(this->service, &result_len);
    VALUE str = icu_ustring_from_uchar_str(result, result_len);
    VALUE rb_str = icu_ustring_to_rb_enc_str(str);
    icu_ustring_clear_ptr(str);
    return rb_str;
}

VALUE transliterator_available_ids(VALUE self)
{
    UErrorCode status = U_ZERO_ERROR;
    UEnumeration* open_ids = utrans_openIDs(&status);
    return icu_enum_to_rb_ary(open_ids, status, 650);
}

void init_icu_transliterator(void)
{
    ID_forward = rb_intern("forward");
    ID_reverse = rb_intern("reverse");

    rb_cICU_Transliterator = rb_define_class_under(rb_mICU, "Transliterator", rb_cObject);
    rb_define_alloc_func(rb_cICU_Transliterator, transliterator_alloc);
    rb_define_method(rb_cICU_Transliterator, "initialize", transliterator_initialize, -1);
    rb_define_method(rb_cICU_Transliterator, "transliterate", transliterator_transliterate, 1);
    rb_define_method(rb_cICU_Transliterator, "unicode_id", transliterator_unicode_id, 0);

    rb_define_module_function(rb_cICU_Transliterator, "available_ids", transliterator_available_ids, 0);
}

#undef GET_TRANSLITERATOR

/* vim: set expandtab sws=4 sw=4: */
