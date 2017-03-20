#include "icu.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"

#define RUBY_C_STRING_TERMINATOR_SIZE 1
#define GET_STRING(_data) icu_uchar_string_data* _data; \
                          TypedData_Get_Struct(self, icu_uchar_string_data, &icu_uchar_string_type, _data)

/* Data types */
typedef struct {
    int32_t len;
    int32_t capa;
    union {
        int old;
        int to;
    } rb_enc_idx;
    UConverter* converter;
    UChar* ptr;
} icu_uchar_string_data;

static void icu_uchar_string_free(void* _this)
{
    icu_uchar_string_data* this = _this;
    ucnv_close(this->converter);
    ruby_xfree(this->ptr);
}

static size_t icu_uchar_string_memsize(const void* data)
{
    icu_uchar_string_data* this = (icu_uchar_string_data*)data;
    size_t size = sizeof(UChar) * this->capa;
    return size + sizeof(icu_uchar_string_data);
}

static const rb_data_type_t icu_uchar_string_type = {
    "icu/uchar_string",
    {NULL, icu_uchar_string_free, icu_uchar_string_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

// avoid name conflicts
#include "ruby/encoding.h"

/* Always allocate the internal string in a C function where you modify it
 * as Ruby GC scans the C stacks and registers to find out GC root
 */
VALUE icu_uchar_string_alloc(void)
{
    icu_uchar_string_data* this;
    return TypedData_Make_Struct(rb_cData, icu_uchar_string_data, &icu_uchar_string_type, this);
}

void icu_uchar_string_replace(VALUE self, VALUE rb_str)
{
    GET_STRING(this);
    UErrorCode status = U_ZERO_ERROR;

    this->rb_enc_idx.old = icu_rb_str_enc_idx(rb_str);
    // take UTF-8 code path
    if (icu_is_rb_enc_idx_as_utf_8(this->rb_enc_idx.old)) {
        this->converter = NULL;
    } else {
        this->converter = ucnv_open(ICU_RB_STRING_ENC_NAME_IDX(this->rb_enc_idx.old), &status);
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
                status = U_ZERO_ERROR;
            } else if (U_FAILURE(status)) {
                rb_raise(rb_eICU_Error, u_errorName(status));
            } else { // retry == true && U_SUCCESS(status)
                break;
            }
        } while (retry);
        this->len = required_capa;
    }
}

void icu_uchar_string_new_capa(VALUE self, int32_t capa)
{
    GET_STRING(this);
    this->capa = capa;
    this->ptr = ALLOC_N(UChar, capa);
}

void icu_uchar_string_set_capa(VALUE self, int32_t capa)
{
    GET_STRING(this);
    this->capa = capa;
    REALLOC_N(this->ptr, UChar, capa);
}

void icu_uchar_string_set_enc(VALUE self, int enc_idx)
{
    GET_STRING(this);

    this->rb_enc_idx.to = enc_idx;
    // take UTF-8 code path
    if (icu_is_rb_enc_idx_as_utf_8(enc_idx)) {
        this->converter = NULL;
    } else {
        UErrorCode status = U_ZERO_ERROR;
        this->converter = ucnv_open(ICU_RB_STRING_ENC_NAME_IDX(this->rb_enc_idx.to), &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    }
}

// used for collecting strings from ICU and converting to Ruby String
void icu_uchar_string_new_capa_enc(VALUE self, int32_t capa, int enc_idx)
{
    icu_uchar_string_set_enc(self, enc_idx);
    icu_uchar_string_new_capa(self, capa);
}

// used for collecting strings from ICU and converting to Ruby String
void icu_uchar_string_set_capa_enc(VALUE self, int32_t capa, int enc_idx)
{
    icu_uchar_string_set_enc(self, enc_idx);
    icu_uchar_string_set_capa(self, capa);
}

VALUE icu_uchar_string_to_rb_enc_str(VALUE self)
{
    GET_STRING(this);
    char* dest;
    int dest_capa;
    int dest_len;
    UErrorCode status = U_ZERO_ERROR;
    if (this->converter == NULL) {
        dest_capa = this->len * 2 + 4;
        dest = ALLOC_N(char, dest_capa);
        u_strToUTF8(dest, dest_capa, &dest_len, this->ptr, this->len, &status);
        if (U_FAILURE(status)) {
            ruby_xfree(dest);
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
        if (dest_len + RUBY_C_STRING_TERMINATOR_SIZE < dest_capa) { // overflow
            REALLOC_N(dest, char, dest_len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
            u_strToUTF8(dest, dest_capa, &dest_len, this->ptr, this->len, &status);
            if (U_FAILURE(status)) {
                ruby_xfree(dest);
                rb_raise(rb_eICU_Error, u_errorName(status));
            }
        }
    } else {
        dest_capa = this->len * 2 + 4;
        dest = ALLOC_N(char, dest_capa);
        int retry = FALSE;
        do {
           dest_len = ucnv_fromUChars(this->converter,
                                      dest,
                                      dest_capa,
                                      this->ptr,
                                      this->len,
                                      &status);
            if (!retry && status == U_BUFFER_OVERFLOW_ERROR) {
                retry = TRUE;
                dest_capa = dest_len + RUBY_C_STRING_TERMINATOR_SIZE;
                REALLOC_N(dest, char, dest_capa);
                status = U_ZERO_ERROR;
            } else if (U_FAILURE(status)) {
                rb_raise(rb_eICU_Error, u_errorName(status));
            } else { // retry == true && U_SUCCESS(status)
                break;
            }
        } while (retry);
    }

    VALUE rb_str = rb_enc_str_new(dest, dest_len, rb_enc_from_index(this->rb_enc_idx.to));
    OBJ_TAINT(rb_str);
    ruby_xfree(dest);
    return rb_str;
}

void* icu_uchar_string_ptr(VALUE self)
{
    GET_STRING(this);
    return this->ptr;
}

int32_t icu_uchar_string_len(VALUE self)
{
    GET_STRING(this);
    return this->len;
}

void icu_uchar_string_set_len(VALUE self, int32_t len)
{
    GET_STRING(this);
    this->len = len;
}

int32_t icu_uchar_string_capa(VALUE self)
{
    GET_STRING(this);
    return this->capa;
}
