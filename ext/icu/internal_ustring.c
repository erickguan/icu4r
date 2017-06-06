#include "icu.h"
#include "unicode/ucnv.h"

// #define ICU_USTRING_DEBUG 1

#define GET_STRING(_data) icu_ustring_data* _data; \
                          TypedData_Get_Struct(self, icu_ustring_data, &icu_ustring_type, _data)
#define GET_STRING_VAL(_val, _data) icu_ustring_data* _data; \
                                    TypedData_Get_Struct(_val, icu_ustring_data, &icu_ustring_type, _data)

VALUE rb_cICU_UString;

/* Data types */
typedef struct {
    int32_t len;
    int32_t capa;
    int rb_enc_idx;
    UConverter* converter;
    UChar* ptr;
} icu_ustring_data;

static void icu_ustring_free(void* _this)
{
    icu_ustring_data* this = _this;
    ucnv_close(this->converter);
    if (this->ptr != NULL) {
        ruby_xfree(this->ptr);
    }
}

static size_t icu_ustring_memsize(const void* data)
{
    icu_ustring_data* this = (icu_ustring_data*)data;
    size_t size = sizeof(UChar) * this->capa;
    return size + sizeof(icu_ustring_data);
}

static const rb_data_type_t icu_ustring_type = {
    "icu/ustring",
    {NULL, icu_ustring_free, icu_ustring_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

// avoid name conflicts
#include "ruby/encoding.h"

/* Always allocate the internal string in a C function where you modify it
 * as Ruby GC scans the C stacks and registers to find out GC root
 */
VALUE icu_ustring_alloc(VALUE self)
{
    icu_ustring_data* this;
    return TypedData_Make_Struct(self, icu_ustring_data, &icu_ustring_type, this);
}

void icu_ustring_resize(VALUE self, int32_t capa)
{
    GET_STRING(this);
    REALLOC_N(this->ptr, UChar, capa);
    this->capa = capa;
}

// some ICU gives a pointer to buffer, used for releasing ptr since we don't manage it
void icu_ustring_clear_ptr(VALUE self)
{
    GET_STRING(this);
    this->ptr = NULL;
}

/*
 Initialize the internal object. Used to convert string to ICU string.
 See also:
   - icu_ustring_init_with_capa_enc
   - icu_ustring_from_uchar_str
*/
VALUE icu_ustring_from_rb_str(VALUE rb_str)
{
    StringValue(rb_str);
    VALUE u_str = icu_ustring_alloc(rb_cICU_UString);
    GET_STRING_VAL(u_str, this);
    UErrorCode status = U_ZERO_ERROR;

    this->rb_enc_idx = icu_rb_str_enc_idx(rb_str);
    // take UTF-8 code path
    if (icu_is_rb_enc_idx_as_utf_8(this->rb_enc_idx)) {
        this->converter = NULL;
    } else {
        this->converter = ucnv_open(ICU_RB_STRING_ENC_NAME_IDX(this->rb_enc_idx), &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    }

    this->capa = RSTRING_LEN(rb_str) + RUBY_C_STRING_TERMINATOR_SIZE;
    this->ptr = ALLOC_N(UChar, this->capa);

#ifdef ICU_USTRING_DEBUG
    printf("icu_ustring_from_rb_str: %p %p %p %p %ld\n", u_str, this->ptr, rb_str, StringValuePtr(rb_str), RSTRING_LEN(rb_str));
#endif

    status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        if (this->converter == NULL) {
            u_strFromUTF8(this->ptr, this->capa, &len,
                          StringValuePtr(rb_str), RSTRING_LEN(rb_str),
                          &status);
        } else {
            len = ucnv_toUChars(this->converter, this->ptr, this->capa,
                                StringValuePtr(rb_str), RSTRING_LEN(rb_str),
                                &status);
        }
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            this->capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            REALLOC_N(this->ptr, UChar, this->capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    this->len = len;

    return u_str;
}

/*
 Initialize the internal object. Used to convert string to Ruby string.
 If the pointer points to a memory space managed by ICU, releasing it before the EOL of our object.
 See also:
   - icu_ustring_init_with_capa_enc
   - icu_ustring_from_uchar_str
   - icu_ustring_clear_ptr
*/
VALUE icu_ustring_from_uchar_str(UChar* str, int32_t len)
{
    if (len <= 0) {
        len = u_strlen(str);
    }
    VALUE u_str = icu_ustring_init_with_capa_enc(len + RUBY_C_STRING_TERMINATOR_SIZE, ICU_RUBY_ENCODING_INDEX);
    GET_STRING_VAL(u_str, this);
    this->ptr = str;
    this->len = len;
    return u_str;
}

/*
 Initialize the internal object. Usually used as a buffer.
 See also:
   - icu_ustring_from_rb_str
   - icu_ustring_from_uchar_str
*/
VALUE icu_ustring_init_with_capa_enc(int32_t capa, int enc)
{
    VALUE buf = icu_ustring_alloc(rb_cICU_UString);
    GET_STRING_VAL(buf, this);
    icu_ustring_set_enc(buf, enc);
    this->capa = capa;
    this->ptr = ALLOC_N(UChar, capa);
    return buf;
}

void icu_ustring_set_enc(VALUE self, int enc_idx)
{
    GET_STRING(this);
    if (this->converter != NULL) {
        ucnv_close(this->converter);
    }

    this->rb_enc_idx = enc_idx;
    // take UTF-8 code path
    if (icu_is_rb_enc_idx_as_utf_8(enc_idx)) {
        this->converter = NULL;
    } else {
        UErrorCode status = U_ZERO_ERROR;
        this->converter = ucnv_open(ICU_RB_STRING_ENC_NAME_IDX(this->rb_enc_idx), &status);
        if (U_FAILURE(status)) {
            rb_raise(rb_eICU_Error, u_errorName(status));
        }
    }
}

VALUE icu_ustring_to_rb_enc_str_with_len(VALUE self, int32_t len)
{
    GET_STRING(this);

#ifdef ICU_USTRING_DEBUG
    printf("icu_ustring_to_rb_enc_str_with_len: %p %ld\n", self, len);
#endif

    this->len = len;
    return icu_ustring_to_rb_enc_str(self);
}

VALUE icu_ustring_to_rb_enc_str(VALUE self)
{
    GET_STRING(this);
    if (this->len < 0) {
        this->len = 0;
    }

#ifdef ICU_USTRING_DEBUG
    printf("icu_ustring_to_rb_enc_str: %p %d %d\n", self, this->len, this->capa);
#endif

    int32_t dest_len;
    int32_t dest_capa = this->len + RUBY_C_STRING_TERMINATOR_SIZE;
    char* dest = ALLOC_N(char, dest_capa);
    int retried = FALSE;
    UErrorCode status = U_ZERO_ERROR;
    do {
        if (this->converter == NULL) {
            u_strToUTF8(dest, dest_capa, &dest_len, this->ptr, this->len, &status);
        } else {
            dest_len = ucnv_fromUChars(this->converter, dest, dest_capa,
                                       this->ptr, this->len,
                                       &status);
        }
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            dest_capa = dest_len + RUBY_C_STRING_TERMINATOR_SIZE;
            REALLOC_N(dest, char, dest_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            ruby_xfree(dest);
            rb_raise(rb_eICU_Error, u_errorName(status));
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);

#ifdef ICU_USTRING_DEBUG
    printf("icu_ustring_to_rb_enc_str (before creating rb str): %p %d %d %d\n", (void *)self, this->len, this->capa, this->rb_enc_idx);
    printf("icu_ustring_to_rb_enc_str (pointers): %p %p %d\n", (void *)self, dest, dest_len);
#endif

    VALUE rb_str = rb_enc_str_new(dest, dest_len, rb_enc_from_index(this->rb_enc_idx));
    ruby_xfree(dest);
    OBJ_TAINT(rb_str);
    return rb_str;
}

UChar* icu_ustring_ptr(VALUE self)
{
    GET_STRING(this);
    return this->ptr;
}

int32_t icu_ustring_len(VALUE self)
{
    GET_STRING(this);
    return this->len;
}

int32_t icu_ustring_capa(VALUE self)
{
    GET_STRING(this);
    return this->capa;
}

#undef GET_STRING
#undef GET_STRING_VAL