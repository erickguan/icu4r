#ifndef RUBY_EXTENSION_ICU_H_
#define RUBY_EXTENSION_ICU_H_

/* System libraries */
#include <stdlib.h>

/* Ruby headers */
#define ONIG_ESCAPE_UCHAR_COLLISION 1  // ruby.h defines UChar macro
#include <ruby.h>
#include <ruby/encoding.h>
#ifdef UChar // fail-safe
  #undef UChar
#endif
#include "unicode/ustring.h"
#include "unicode/uenum.h"

/* Globals */

extern VALUE rb_mICU;
extern VALUE rb_eICU_Error;
extern VALUE rb_cICU_UString;
extern VALUE rb_cICU_Collator;
extern VALUE rb_cICU_Normalizer;
extern VALUE rb_cICU_SpoofChecker;
extern VALUE rb_cICU_Transliterator;
extern VALUE rb_cICU_CharsetDetector;
extern VALUE rb_cICU_CharsetDetector_Match;
extern VALUE rb_cICU_Locale;

/* Prototypes */
void Init_icu                                          _(( void ));
void init_internal_encoding                            _(( void ));
void init_rb_errors                                    _(( void ));
void init_icu_collator                                 _(( void ));
void init_icu_normalizer                               _(( void ));
void init_icu_spoof_checker                            _(( void ));
void init_icu_transliterator                           _(( void ));
void init_icu_charset_detector                         _(( void ));
void init_icu_locale                                   _(( void ));

int icu_is_rb_enc_idx_as_utf_8                         _(( int ));
int icu_is_rb_str_as_utf_8                             _(( VALUE ));
const char* icu_rb_str_enc_name                        _(( int ));
VALUE rb_str_enc_to_ascii_as_utf8                      _(( VALUE ));
int icu_rb_str_enc_idx                                 _(( VALUE ));
VALUE icu_enum_to_rb_ary                               _(( UEnumeration*, UErrorCode, long ));
extern void icu_rb_raise_icu_error                     _(( UErrorCode ));
extern void icu_rb_raise_icu_parse_error               _(( const UParseError* ));
extern void icu_rb_raise_icu_invalid_parameter         _(( const char*, const char* ));

VALUE icu_ustring_init_with_capa_enc                   _(( int32_t, int ));
VALUE icu_ustring_from_rb_str                          _(( VALUE ));
VALUE icu_ustring_from_uchar_str                       _(( const UChar*, int32_t ));
void icu_ustring_clear_ptr                             _(( VALUE ));
void icu_ustring_resize                                _(( VALUE, int32_t ));
void icu_ustring_set_enc                               _(( VALUE, int ));
VALUE icu_ustring_to_rb_enc_str_with_len               _(( VALUE, int32_t ));
VALUE icu_ustring_to_rb_enc_str                        _(( VALUE ));
UChar* icu_ustring_ptr                                 _(( VALUE ));
int32_t icu_ustring_len                                _(( VALUE ));
int32_t icu_ustring_capa                               _(( VALUE ));
VALUE char_buffer_to_rb_str                            _(( const char* ));
char* char_buffer_new                                  _(( int32_t ));
void char_buffer_resize                                _(( const char*, int32_t ));
void char_buffer_free                                  _(( const char* ));

/* Constants */
#define RUBY_C_STRING_TERMINATOR_SIZE 1

/* Macros */
#define ICU_RUBY_ENCODING_INDEX (rb_enc_to_index(rb_default_internal_encoding()) || rb_locale_encindex())
#define ICU_RB_STRING_ENC_NAME_IDX(_idx) rb_enc_from_index(_idx) != NULL ? (rb_enc_from_index(_idx))->name : ""

#endif // RUBY_EXTENSION_ICU_H_

/* vim: set expandtab sws=4 sw=4: */
