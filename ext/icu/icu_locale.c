#include "icu.h"
#include "unicode/uloc.h"
#include <string.h>
#include <stdlib.h>

VALUE rb_cICU_Locale;
static ID ID_ltr;
static ID ID_rtl;
static ID ID_ttb;
static ID ID_btt;
static ID ID_unknown;

VALUE locale_initialize(VALUE self, VALUE id)
{
    id = rb_str_enc_to_ascii_as_utf8(id);
    rb_iv_set(self, "@id", id);
    return self;
}

inline static VALUE locale_new_from_cstr(const char* str)
{
    VALUE rb_str = rb_str_new_cstr(str);
    return locale_initialize(rb_obj_alloc(rb_cICU_Locale), rb_str_enc_to_ascii_as_utf8(rb_str));
}

inline static VALUE char_buffer_to_rb_str(const char* buffer)
{
    VALUE str = rb_str_new_cstr(buffer);
    return rb_str_export_to_enc(str, rb_enc_from_index(ICU_RUBY_ENCODING_INDEX));
}

inline static char* char_buffer_new(int32_t buf_size)
{
    char* buffer = ALLOC_N(char, buf_size);
    return buffer;
}

inline static void char_buffer_resize(const char* buffer, int32_t buf_size)
{
    REALLOC_N(buffer, char, buf_size);
}

inline static void char_buffer_free(const char* buffer)
{
    ruby_xfree((void*)buffer);
}

VALUE locale_singleton_available(VALUE klass)
{
    int32_t len = uloc_countAvailable();
    VALUE result = rb_ary_new2(len);
    for (int32_t i = 0; i < len; ++i) {
        rb_ary_push(result, locale_new_from_cstr(uloc_getAvailable(i)));
    }
    return result;
}

static inline VALUE locale_singleton_get_default_internal(void)
{
    const char* locale = uloc_getDefault();
    return locale_new_from_cstr(locale);
}

VALUE locale_singleton_get_default(VALUE klass)
{
    return locale_singleton_get_default_internal();
}

VALUE locale_singleton_set_default(VALUE klass, VALUE val)
{
    val = rb_str_enc_to_ascii_as_utf8(val);
    UErrorCode status = U_ZERO_ERROR;
    uloc_setDefault(RSTRING_PTR(val), &status);
    if (U_FAILURE(status)) {
        icu_rb_raise_icu_error(status);
    }
    return locale_singleton_get_default_internal();
}

VALUE locale_singleton_for_language_tag(VALUE klass, VALUE tag)
{
    tag = rb_str_enc_to_ascii_as_utf8(tag);
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_forLanguageTag(RSTRING_PTR(tag),
                                  buffer,
                                  buffer_capa,
                                  NULL /* parsedLength of tag */,
                                  &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE loc = locale_new_from_cstr(buffer);
    char_buffer_free(buffer);

    return loc;
}

VALUE locale_singleton_for_lcid(VALUE klass, VALUE lcid)
{
    uint32_t host_id = NUM2UINT(lcid);
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getLocaleForLCID(host_id,
                                    buffer,
                                    buffer_capa,
                                    &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = locale_new_from_cstr(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_singleton_iso_countries(VALUE klass)
{
    const char* const* ary = uloc_getISOCountries();
    VALUE result = rb_ary_new2(250); // the number of countries now is 249.
    while (*ary != NULL) {
        // NUL-terminated C string allocated by ICU, so use the buffer function
        rb_ary_push(result, char_buffer_to_rb_str(*ary));
        ary++;
    }
    return result;
}

VALUE locale_singleton_iso_languages(VALUE klass)
{
    const char* const* ary = uloc_getISOLanguages();
    VALUE result = rb_ary_new2(250); // the number of countries now is 249.
    while (*ary != NULL) {
        // NUL-terminated C string allocated by ICU, so use the buffer function
        rb_ary_push(result, char_buffer_to_rb_str(*ary));
        ary++;
    }
    return result;
}

VALUE locale_language_tag(int argc, VALUE* argv, VALUE self)
{
    VALUE strict;
    rb_scan_args(argc, argv, "01", &strict);
    if (strict != Qtrue) {
        strict = Qfalse;
    }

    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_toLanguageTag(RSTRING_PTR(id),
                                 buffer,
                                 buffer_capa,
                                 strict == Qtrue ? TRUE : FALSE,
                                 &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE loc = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return loc;
}

VALUE locale_lcid(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    return ULONG2NUM(uloc_getLCID(RSTRING_PTR(id)));
}

VALUE locale_display_country(int argc, VALUE* argv, VALUE self)
{
    VALUE display_locale;
    rb_scan_args(argc, argv, "01", &display_locale);
    if (!NIL_P(display_locale)) {
        display_locale = rb_str_enc_to_ascii_as_utf8(display_locale);
    }

    VALUE id = rb_iv_get(self, "@id");
    VALUE buffer = icu_ustring_init_with_capa_enc(64, ICU_RUBY_ENCODING_INDEX);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;

    do {
        len = uloc_getDisplayCountry(RSTRING_PTR(id),
                                     NIL_P(display_locale) ? NULL : RSTRING_PTR(display_locale),
                                     icu_ustring_ptr(buffer),
                                     icu_ustring_capa(buffer),
                                     &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(buffer, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    return icu_ustring_to_rb_enc_str_with_len(buffer, len);
}

VALUE locale_display_language(int argc, VALUE* argv, VALUE self)
{
    VALUE display_locale;
    rb_scan_args(argc, argv, "01", &display_locale);
    // if use NULL instead of the default, ICU fails which doesn't align with the doc at 59.1
    display_locale = rb_str_enc_to_ascii_as_utf8((NIL_P(display_locale) ?
                                                      locale_singleton_get_default_internal() :
                                                      display_locale));

    VALUE id = rb_iv_get(self, "@id");
    VALUE buffer = icu_ustring_init_with_capa_enc(64, ICU_RUBY_ENCODING_INDEX);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getDisplayLanguage(RSTRING_PTR(id),
                                      RSTRING_PTR(display_locale),
                                      icu_ustring_ptr(buffer),
                                      icu_ustring_capa(buffer),
                                      &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(buffer, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    return icu_ustring_to_rb_enc_str_with_len(buffer, len);
}

VALUE locale_display_name(int argc, VALUE* argv, VALUE self)
{
    VALUE display_locale;
    rb_scan_args(argc, argv, "01", &display_locale);
    if (!NIL_P(display_locale)) {
        display_locale = rb_str_enc_to_ascii_as_utf8(display_locale);
    }

    VALUE id = rb_iv_get(self, "@id");
    VALUE buffer = icu_ustring_init_with_capa_enc(64, ICU_RUBY_ENCODING_INDEX);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getDisplayName(RSTRING_PTR(id),
                                  NIL_P(display_locale) ? NULL : RSTRING_PTR(display_locale),
                                  icu_ustring_ptr(buffer),
                                  icu_ustring_capa(buffer),
                                  &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(buffer, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    return icu_ustring_to_rb_enc_str_with_len(buffer, len);
}

VALUE locale_display_script(int argc, VALUE* argv, VALUE self)
{
    VALUE display_locale;
    rb_scan_args(argc, argv, "01", &display_locale);
    if (!NIL_P(display_locale)) {
        display_locale = rb_str_enc_to_ascii_as_utf8(display_locale);
    }

    VALUE id = rb_iv_get(self, "@id");
    VALUE buffer = icu_ustring_init_with_capa_enc(64, ICU_RUBY_ENCODING_INDEX);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getDisplayScript(RSTRING_PTR(id),
                                    NIL_P(display_locale) ? NULL : RSTRING_PTR(display_locale),
                                    icu_ustring_ptr(buffer),
                                    icu_ustring_capa(buffer),
                                    &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(buffer, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    return icu_ustring_to_rb_enc_str_with_len(buffer, len);
}

VALUE locale_display_variant(int argc, VALUE* argv, VALUE self)
{
    VALUE display_locale;
    rb_scan_args(argc, argv, "01", &display_locale);
    if (!NIL_P(display_locale)) {
        display_locale = rb_str_enc_to_ascii_as_utf8(display_locale);
    }

    VALUE id = rb_iv_get(self, "@id");
    VALUE buffer = icu_ustring_init_with_capa_enc(64, ICU_RUBY_ENCODING_INDEX);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getDisplayVariant(RSTRING_PTR(id),
                                     NIL_P(display_locale) ? NULL : RSTRING_PTR(display_locale),
                                     icu_ustring_ptr(buffer),
                                     icu_ustring_capa(buffer),
                                     &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            icu_ustring_resize(buffer, len + RUBY_C_STRING_TERMINATOR_SIZE);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    return icu_ustring_to_rb_enc_str_with_len(buffer, len);
}

VALUE locale_name(VALUE self)
{
    int32_t buffer_capa = 64;
    VALUE id = rb_iv_get(self, "@id");
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getName(RSTRING_PTR(id),
                           buffer,
                           buffer_capa,
                           &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_base_name(VALUE self)
{
    int32_t buffer_capa = 64;
    VALUE id = rb_iv_get(self, "@id");
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getBaseName(RSTRING_PTR(id),
                               buffer,
                               buffer_capa,
                               &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_canonical_name(VALUE self)
{
    int32_t buffer_capa = 64;
    VALUE id = rb_iv_get(self, "@id");
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_canonicalize(RSTRING_PTR(id),
                                buffer,
                                buffer_capa,
                                &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_parent(VALUE self)
{
    int32_t buffer_capa = 64;
    VALUE id = rb_iv_get(self, "@id");
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getParent(RSTRING_PTR(id),
                             buffer,
                             buffer_capa,
                             &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_iso_country(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    return rb_str_new_cstr(uloc_getISO3Country(RSTRING_PTR(id)));
}

VALUE locale_iso_language(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    return rb_str_new_cstr(uloc_getISO3Language(RSTRING_PTR(id)));
}

VALUE locale_keyword(VALUE self, VALUE keyword)
{
    keyword = rb_str_enc_to_ascii_as_utf8(keyword);
    int32_t buffer_capa = 64;
    VALUE id = rb_iv_get(self, "@id");
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getKeywordValue(RSTRING_PTR(id),
                                   RSTRING_PTR(keyword),
                                   buffer,
                                   buffer_capa,
                                   &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_keywords(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    UErrorCode status = U_ZERO_ERROR;
    UEnumeration* result = uloc_openKeywords(RSTRING_PTR(id), &status);
    return icu_enum_to_rb_ary(result, status, 3);
}

// TODO: check the keyword and value
VALUE locale_with_keyword(VALUE self, VALUE keyword, VALUE value)
{
    keyword = rb_str_enc_to_ascii_as_utf8(keyword);
    int32_t len_keyword = RSTRING_LENINT(keyword);
    if (len_keyword == 0) {
        rb_raise(rb_eArgError, "invalid value for keyword: %+"PRIsVALUE, self);
    }
    if (!NIL_P(value)) {
        value = rb_str_enc_to_ascii_as_utf8(value);
    }

    VALUE id = rb_iv_get(self, "@id");
    int32_t len_id = RSTRING_LENINT(id);
    int32_t buffer_capa = 64 + len_id + len_keyword + (NIL_P(value) ? 0 : RSTRING_LENINT(value));
    char* buffer = char_buffer_new(buffer_capa);
    memmove(buffer, RSTRING_PTR(id), len_id);
    buffer[len_id] = '\0';

    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t buffer_required;
    do {
        buffer_required = uloc_setKeywordValue(RSTRING_PTR(keyword),
                                               NIL_P(value) ? NULL : RSTRING_PTR(value),
                                               buffer,
                                               buffer_capa,
                                               &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = buffer_required;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    // NUL is inserted by ICU when buffer is enough

    VALUE res = locale_new_from_cstr(buffer);
    char_buffer_free(buffer);
    return res;
}

static inline VALUE locale_layout_symbol(ULayoutType result)
{
    switch (result) {
    case ULOC_LAYOUT_LTR:
        return ID2SYM(ID_ltr);
    case ULOC_LAYOUT_RTL:
        return ID2SYM(ID_rtl);
    case ULOC_LAYOUT_TTB:
        return ID2SYM(ID_ttb);
    case ULOC_LAYOUT_BTT:
        return ID2SYM(ID_btt);
    case ULOC_LAYOUT_UNKNOWN: default:
        return ID2SYM(ID_unknown);
    }
}

VALUE locale_character_orientation(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    UErrorCode status = U_ZERO_ERROR;
    ULayoutType result = uloc_getCharacterOrientation(RSTRING_PTR(id), &status);
    return locale_layout_symbol(result);
}

VALUE locale_line_orientation(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    UErrorCode status = U_ZERO_ERROR;
    ULayoutType result = uloc_getLineOrientation(RSTRING_PTR(id), &status);
    return locale_layout_symbol(result);
}

VALUE locale_country(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getCountry(RSTRING_PTR(id),
                              buffer,
                              buffer_capa,
                              &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE loc = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return loc;
}

VALUE locale_language(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getLanguage(RSTRING_PTR(id),
                               buffer,
                               buffer_capa,
                               &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE loc = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return loc;
}

VALUE locale_script(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getScript(RSTRING_PTR(id),
                             buffer,
                             buffer_capa,
                             &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_variant(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_getVariant(RSTRING_PTR(id),
                              buffer,
                              buffer_capa,
                              &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = char_buffer_to_rb_str(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_with_likely_subtags(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_addLikelySubtags(RSTRING_PTR(id),
                                    buffer,
                                    buffer_capa,
                                    &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = locale_new_from_cstr(buffer);
    char_buffer_free(buffer);
    return res;
}

VALUE locale_with_minimized_subtags(VALUE self)
{
    VALUE id = rb_iv_get(self, "@id");
    int32_t buffer_capa = 64;
    char* buffer = char_buffer_new(buffer_capa);
    UErrorCode status = U_ZERO_ERROR;
    int retried = FALSE;
    int32_t len;
    do {
        len = uloc_minimizeSubtags(RSTRING_PTR(id),
                                   buffer,
                                   buffer_capa,
                                   &status);
        if (!retried && status == U_BUFFER_OVERFLOW_ERROR) {
            retried = TRUE;
            buffer_capa = len + RUBY_C_STRING_TERMINATOR_SIZE;
            char_buffer_resize(buffer, buffer_capa);
            status = U_ZERO_ERROR;
        } else if (U_FAILURE(status)) {
            char_buffer_free(buffer);
            icu_rb_raise_icu_error(status);
        } else { // retried == true && U_SUCCESS(status)
            break;
        }
    } while (retried);
    buffer[len] = '\0';

    VALUE res = locale_new_from_cstr(buffer);
    char_buffer_free(buffer);
    return res;
}

void init_icu_locale(void)
{
    ID_ltr = rb_intern("ltr");
    ID_rtl = rb_intern("rtl");
    ID_ttb = rb_intern("ttb");
    ID_btt = rb_intern("btt");
    ID_unknown = rb_intern("unknown");

    rb_cICU_Locale = rb_define_class_under(rb_mICU, "Locale", rb_cObject);
    rb_define_singleton_method(rb_cICU_Locale, "available", locale_singleton_available, 0);
    rb_define_singleton_method(rb_cICU_Locale, "default", locale_singleton_get_default, 0);
    rb_define_singleton_method(rb_cICU_Locale, "default=", locale_singleton_set_default, 1);
    rb_define_singleton_method(rb_cICU_Locale, "for_language_tag", locale_singleton_for_language_tag, 1);
    rb_define_singleton_method(rb_cICU_Locale, "for_lcid", locale_singleton_for_lcid, 1);
    rb_define_singleton_method(rb_cICU_Locale, "iso_countries", locale_singleton_iso_countries, 0);
    rb_define_singleton_method(rb_cICU_Locale, "iso_languages", locale_singleton_iso_languages, 0);
    rb_define_method(rb_cICU_Locale, "initialize", locale_initialize, 1);
    rb_define_method(rb_cICU_Locale, "language_tag", locale_language_tag, -1);
    rb_define_method(rb_cICU_Locale, "lcid", locale_lcid, 0);
    rb_define_method(rb_cICU_Locale, "display_country", locale_display_country, -1);
    rb_define_method(rb_cICU_Locale, "display_language", locale_display_language, -1);
    rb_define_method(rb_cICU_Locale, "display_name", locale_display_name, -1);
    rb_define_method(rb_cICU_Locale, "display_script", locale_display_script, -1);
    rb_define_method(rb_cICU_Locale, "display_variant", locale_display_variant, -1);
    rb_define_method(rb_cICU_Locale, "name", locale_name, 0);
    rb_define_method(rb_cICU_Locale, "base_name", locale_base_name, 0);
    rb_define_method(rb_cICU_Locale, "canonical_name", locale_canonical_name, 0);
    rb_define_method(rb_cICU_Locale, "parent", locale_parent, 0);
    rb_define_method(rb_cICU_Locale, "iso_country", locale_iso_country, 0);
    rb_define_method(rb_cICU_Locale, "iso_language", locale_iso_language, 0);
    rb_define_method(rb_cICU_Locale, "keyword", locale_keyword, 1);
    rb_define_method(rb_cICU_Locale, "keywords", locale_keywords, 0);
    rb_define_method(rb_cICU_Locale, "with_keyword", locale_with_keyword, 2);
    rb_define_method(rb_cICU_Locale, "character_orientation", locale_character_orientation, 0);
    rb_define_method(rb_cICU_Locale, "line_orientation", locale_line_orientation, 0);
    rb_define_method(rb_cICU_Locale, "country", locale_country, 0);
    rb_define_method(rb_cICU_Locale, "language", locale_language, 0);
    rb_define_method(rb_cICU_Locale, "script", locale_script, 0);
    rb_define_method(rb_cICU_Locale, "variant", locale_variant, 0);
    rb_define_method(rb_cICU_Locale, "with_likely_subtags", locale_with_likely_subtags, 0);
    rb_define_method(rb_cICU_Locale, "with_minimized_subtags", locale_with_minimized_subtags, 0);

}

/* vim: set expandtab sws=4 sw=4: */
