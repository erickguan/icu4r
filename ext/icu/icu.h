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

extern VALUE rb_mICU;
extern VALUE rb_eICU_Error;
extern VALUE rb_cICU_UString;
extern VALUE rb_cICU_Collator;
extern VALUE rb_cICU_Normalizer;
extern VALUE rb_cICU_SpoofChecker;
extern VALUE rb_cICU_Transliterator;

/* Prototypes */
void Init_icu                                          _(( void ));
void init_icu_errors                                   _(( void ));
void init_icu_collator                                 _(( void ));
void init_icu_normalizer                               _(( void ));
void init_icu_spoof_checker                            _(( void ));
void init_icu_transliterator                           _(( void ));

int icu_is_rb_enc_idx_as_utf_8                         _(( int ));
int icu_is_rb_str_as_utf_8                             _(( VALUE ));
const char* icu_rb_str_enc_name                        _(( int ));
int icu_rb_str_enc_idx                                 _(( VALUE ));

VALUE icu_ustring_init_with_capa_enc                   _(( int32_t, int ));
VALUE icu_ustring_from_rb_str                          _(( VALUE ));
VALUE icu_ustring_from_uchar_str                       _(( UChar*, int32_t ));
void icu_ustring_clear_ptr                             _(( VALUE ));
void icu_ustring_resize                                _(( VALUE, int32_t ));
void icu_ustring_set_enc                               _(( VALUE, int ));
VALUE icu_ustring_to_rb_enc_str_with_len               _(( VALUE, int32_t ));
VALUE icu_ustring_to_rb_enc_str                        _(( VALUE ));
UChar* icu_ustring_ptr                                 _(( VALUE ));
int32_t icu_ustring_len                                _(( VALUE ));
int32_t icu_ustring_capa                               _(( VALUE ));

/* Constants */
#define RUBY_C_STRING_TERMINATOR_SIZE 1

/* Macros */
#define ICU_RUBY_ENCODING_INDEX (rb_enc_to_index(rb_default_internal_encoding()) || rb_locale_encindex())
#define ICU_RB_STRING_ENC_NAME_IDX(_idx) rb_enc_from_index(_idx) != NULL ? (rb_enc_from_index(_idx))->name : ""

#endif // RUBY_EXTENSION_ICU_H_

/* vim: set expandtab sws=4 sw=4: */
