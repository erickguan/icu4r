#include "icu.h"
#include "unicode/unorm2.h"

#define GET_NORMALIZER(_data) icu_normalizer_data* _data; \
                              TypedData_Get_Struct(self, icu_normalizer_data, &icu_normalizer_type, _data)

VALUE rb_cICU_Normalizer;
static ID ID_nfc;
static ID ID_nfkc;
static ID ID_nfkc_cf;
static ID ID_compose;
static ID ID_decompose;

typedef struct {
    VALUE rb_instance;
    int customized;
    UNormalizer2* service;
} icu_normalizer_data;

static void normalizer_free(void* _this)
{
    icu_normalizer_data* this = _this;
    if (this->customized == TRUE) { // If it's not constructed instance, shall not to be closed.
        unorm2_close(this->service);
    }
}

static size_t normalizer_memsize(const void* _)
{
    return sizeof(icu_normalizer_data);
}

static const rb_data_type_t icu_normalizer_type = {
    "icu/normalizer",
    {NULL, normalizer_free, normalizer_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE normalizer_alloc(VALUE self)
{
    icu_normalizer_data* this;
    return TypedData_Make_Struct(self, icu_normalizer_data, &icu_normalizer_type, this);
}

VALUE normalizer_initialize(int argc, VALUE* argv, VALUE self)
{
    VALUE sym_name;
    VALUE sym_mode;
    rb_scan_args(argc, argv, "02", &sym_name, &sym_mode);
    if (NIL_P(sym_name)) {
        sym_name = ID2SYM(ID_nfc);
    }
    if (NIL_P(sym_mode)) {
        sym_mode = ID2SYM(ID_decompose);
    }
    int mode = UNORM2_DECOMPOSE;
    if (sym_mode == ID2SYM(ID_compose)) {
        mode = UNORM2_COMPOSE;
    }
    GET_NORMALIZER(this);
    this->rb_instance = self;
    this->customized = FALSE;

    UErrorCode status = U_ZERO_ERROR;
    this->service = unorm2_getInstance(NULL,
                                       rb_id2name(SYM2ID(sym_name)),
                                       mode,
                                       &status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    return self;
}

VALUE normalizer_normalize(VALUE self, VALUE rb_str)
{
    StringValue(rb_str);
    GET_NORMALIZER(this);
    VALUE in = icu_ustring_from_rb_str(rb_str);
    VALUE out = icu_ustring_init_with_capa_enc(RSTRING_LENINT(rb_str) * 2 + RUBY_C_STRING_TERMINATOR_SIZE, ICU_RUBY_ENCODING_INDEX);

    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = unorm2_normalize(this->service,
                               icu_ustring_ptr(in), icu_ustring_len(in),
                               icu_ustring_ptr(out), icu_ustring_capa(out),
                               &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(out, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);

    return icu_ustring_to_rb_enc_str_with_len(out, len);
}

void init_icu_normalizer(void)
{
    ID_nfc = rb_intern("nfc");
    ID_nfkc = rb_intern("nfkc");
    ID_nfkc_cf = rb_intern("nfkc_cf");
    ID_compose = rb_intern("compose");
    ID_decompose = rb_intern("decompose");

    rb_cICU_Normalizer = rb_define_class_under(rb_mICU, "Normalizer", rb_cObject);
    rb_define_alloc_func(rb_cICU_Normalizer, normalizer_alloc);
    rb_define_method(rb_cICU_Normalizer, "initialize", normalizer_initialize, -1);
    rb_define_method(rb_cICU_Normalizer, "normalize", normalizer_normalize, 1);
}

#undef GET_NORMALIZER

/* vim: set expandtab sws=4 sw=4: */
