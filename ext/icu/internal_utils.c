#include "icu.h"
#include "unicode/utypes.h"

VALUE icu_enum_to_rb_ary(UEnumeration* icu_enum, UErrorCode status, long pre_allocated)
{
    if (U_FAILURE(status)) {
        uenum_close(icu_enum);
        icu_rb_raise_icu_error(status);
    }
    VALUE result = rb_ary_new2(pre_allocated);
    const UChar* ptr = NULL;
    int32_t len = 0;
    status = U_ZERO_ERROR;
    while ((ptr = uenum_unext(icu_enum, &len, &status)) != NULL) {
        if (U_FAILURE(status)) {
            uenum_close(icu_enum);
            icu_rb_raise_icu_error(status);
        }
        VALUE s = icu_ustring_from_uchar_str(ptr, len);
        rb_ary_push(result, icu_ustring_to_rb_enc_str(s));
        icu_ustring_clear_ptr(s);
        status = U_ZERO_ERROR;
    }
    uenum_close(icu_enum);
    return result;
}

extern inline void icu_rb_raise_icu_error(UErrorCode status)
{
    rb_raise(rb_eICU_Error, "ICU Error Code: %d, %s.", status, u_errorName(status));
}


extern inline void
icu_rb_raise_icu_invalid_parameter(const char* parameter,
                                   const char* error_message)
{
    rb_raise(rb_eICU_InvalidParameterError,
             "ICU Invalid parameter: %s, %s.",
             parameter,
             error_message);
}


extern inline void icu_rb_raise_icu_parse_error(const UParseError* error)
{
    rb_raise(rb_eICU_InvalidParameterError,
             "ICU Parse Error: Line %d, offset %d.",
             error->line,
             error->offset);
}
