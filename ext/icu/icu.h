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
int  icu_choose_encoding                               _(( void ));

/* Macros */
#define ICU_RUBY_ENCODING_INDEX(_idx) (_idx) = (rb_enc_to_index(rb_default_internal_encoding()) || rb_locale_encindex())


#endif // RUBY_EXTENSION_ICU_H_

/* vim: set expandtab sws=4 sw=4: */
