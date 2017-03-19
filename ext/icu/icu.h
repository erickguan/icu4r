#ifndef RUBY_EXTENSION_ICU_H_
#define RUBY_EXTENSION_ICU_H_

/* System libraries */
#include <stdlib.h>

/* Ruby headers */
#include "ruby.h"

/* Globals */

extern VALUE rb_mICU;
extern VALUE rb_eICU_Error;
extern VALUE rb_cICU_Collator;

/* Prototypes */
void Init_icu                                          _(( void ));
void init_icu_errors                                   _(( void ));
void init_icu_collator                                 _(( void ));

int icu_choose_encoding                                _(( void ));
int icu_is_rb_enc_idx_as_utf_8                         _(( int ));
int icu_is_rb_str_as_utf_8                             _(( VALUE ));
const char* icu_rb_str_enc_name                        _(( int ));
int icu_rb_str_enc_idx                                 _(( VALUE ));
VALUE icu_uchar_string_alloc                           _(( void ));
VALUE icu_uchar_string_new                             _(( VALUE ));
const void* icu_uchar_string_ptr                       _(( VALUE ));
int32_t icu_uchar_string_len                           _(( VALUE ));
static void icu_uchar_string_free                      _(( void* ));
static size_t icu_uchar_string_memsize                 _(( const void* ));

/* Macros */
#define ICU_RB_STRING_ENC_NAME_IDX(_idx) rb_enc_from_index(_idx) != NULL ? (rb_enc_from_index(_idx))->name : ""

#endif // RUBY_EXTENSION_ICU_H_

/* vim: set expandtab sws=4 sw=4: */
