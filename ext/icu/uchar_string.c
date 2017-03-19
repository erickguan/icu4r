#include "icu.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"

/* Data types */
typedef struct {
    int32_t len;
    int32_t capa;
    int rb_old_enc_idx;
    UConverter* converter;
    UChar* ptr;
} icu_uchar_string_data;

static const rb_data_type_t icu_uchar_string_type = {
    "icu/uchar_string",
    {NULL, icu_uchar_string_free, icu_uchar_string_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

// avoid name conflicts
#include "ruby/encoding.h"

VALUE icu_uchar_string_alloc(void)
{
    icu_uchar_string_data* this;
    return TypedData_Make_Struct(rb_cData, icu_uchar_string_data, &icu_uchar_string_type, this);
}

VALUE icu_uchar_string_new(VALUE rb_str)
{
    // StringValue(rb_str);
    VALUE str = icu_uchar_string_alloc();
    icu_uchar_string_data* this;
    TypedData_Get_Struct(str, icu_uchar_string_data, &icu_uchar_string_type, this);
    UErrorCode status = U_ZERO_ERROR;

    this->rb_old_enc_idx = icu_rb_str_enc_idx(rb_str);
    // take UTF-8 code path
    if (icu_is_rb_enc_idx_as_utf_8(this->rb_old_enc_idx)) {
        this->converter = NULL;
    } else {
        this->converter = ucnv_open(ICU_RB_STRING_ENC_NAME_IDX(this->rb_old_enc_idx), &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    }

    this->capa = RSTRING_LEN(rb_str) + 1; // TODO: reconsider initial size
    this->ptr = ALLOC_N(UChar, this->capa);
    status = U_ZERO_ERROR;

    if (this->converter == NULL) {
        u_strFromUTF8Lenient(this->ptr,
                             this->capa,
                             &(this->len),
                             RSTRING_PTR(rb_str),
                             RSTRING_LEN(rb_str),
                             &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    } else {
        int retry = FALSE;
        int32_t required_capa;
        do {
            required_capa = ucnv_toUChars(this->converter,
                                          this->ptr,
                                          this->capa,
                                          RSTRING_PTR(rb_str),
                                          RSTRING_LEN(rb_str),
                                          &status);
            if (!retry && status == U_BUFFER_OVERFLOW_ERROR) {
                retry = TRUE;
                this->capa = required_capa + 2;
                REALLOC_N(this->ptr, UChar, this->capa);
            } else if (U_FAILURE(status)) {
                rb_raise(rb_eICU_Error, u_errorName(status));
            } else { // retry == true && U_SUCCESS(status)
                break;
            }
        } while (retry);
        this->len = required_capa;
    }

    return str;
}

const void* icu_uchar_string_ptr(VALUE self)
{
    icu_uchar_string_data* this;
    TypedData_Get_Struct(self, icu_uchar_string_data, &icu_uchar_string_type, this);
    return this->ptr;
}

int32_t icu_uchar_string_len(VALUE self)
{
    icu_uchar_string_data* this;
    TypedData_Get_Struct(self, icu_uchar_string_data, &icu_uchar_string_type, this);
    return this->len;
}


static void icu_uchar_string_free(void* _this)
{
    icu_uchar_string_data* this = _this;
    ucnv_close(this->converter);
    ruby_xfree(this->ptr);
}

static size_t icu_uchar_string_memsize(const void* data)
{
    icu_uchar_string_data* this = data;
    size_t size = sizeof(UChar) * this->capa;
    return size + sizeof(icu_uchar_string_data);
}
