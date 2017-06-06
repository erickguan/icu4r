#ifndef RUBY_EXTENSION_ICU_H_
#define RUBY_EXTENSION_ICU_H_

/* System libraries */
#include <stdlib.h>

/* Ruby headers */
#define ONIG_ESCAPE_UCHAR_COLLISION 1
#include "ruby.h"
#undef UChar
#include "unicode/ustring.h"

/* Globals */

#define RUBY_C_STRING_TERMINATOR_SIZE 1

extern VALUE rb_mICU;
extern VALUE rb_eICU_Error;
extern VALUE rb_cICU_UCharString;
extern VALUE rb_cICU_Collator;
extern VALUE rb_cICU_Normalizer;

/* Prototypes */
void Init_icu                                          _(( void ));
void init_icu_errors                                   _(( void ));
void init_icu_collator                                 _(( void ));
void init_icu_normalizer                               _(( void ));

int icu_is_rb_enc_idx_as_utf_8                         _(( int ));
int icu_is_rb_str_as_utf_8                             _(( VALUE ));
const char* icu_rb_str_enc_name                        _(( int ));
int icu_rb_str_enc_idx                                 _(( VALUE ));
void  icu_uchar_string_clear_ptr                       _(( VALUE ));
VALUE icu_uchar_string_alloc                           _(( VALUE ));
VALUE icu_uchar_string_replace                         _(( VALUE, VALUE ));
VALUE icu_uchar_string_new_buf_capa_enc                _(( int32_t, int ));
VALUE icu_uchar_string_from_rb_str                     _(( VALUE ));
VALUE icu_uchar_string_from_uchar_str                  _(( UChar*, int32_t ));
void icu_uchar_string_set_uchar                        _(( VALUE, UChar*, int32_t ));
void icu_uchar_string_new_capa_enc                     _(( VALUE, int32_t, int ));
void icu_uchar_string_set_capa_enc                     _(( VALUE, int32_t, int ));
void icu_uchar_string_new_capa                         _(( VALUE, int32_t ));
void icu_uchar_string_set_capa                         _(( VALUE, int32_t ));
void icu_uchar_string_set_enc                          _(( VALUE, int ));
VALUE icu_uchar_string_to_rb_enc_str                   _(( VALUE ));
void* icu_uchar_string_ptr                             _(( VALUE ));
int32_t icu_uchar_string_len                           _(( VALUE ));
void icu_uchar_string_set_len                          _(( VALUE, int32_t ));
int32_t icu_uchar_string_capa                          _(( VALUE ));

/* Macros */
#define ICU_RUBY_ENCODING_INDEX (rb_enc_to_index(rb_default_internal_encoding()) || rb_locale_encindex())
#define ICU_RB_STRING_ENC_NAME_IDX(_idx) rb_enc_from_index(_idx) != NULL ? (rb_enc_from_index(_idx))->name : ""

#endif // RUBY_EXTENSION_ICU_H_

/* vim: set expandtab sws=4 sw=4: */
