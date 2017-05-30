#include "icu.h"
#include "unicode/uspoof.h"

#define GET_SPOOF_CHECKER(_data) icu_spoof_checker_data* _data; \
                                 TypedData_Get_Struct(self, icu_spoof_checker_data, &icu_spoof_checker_data, _data)

VALUE rb_cICU_SpoofChecker;

typedef struct {
    VALUE instance;
    USpoofChecker* checker;
} icu_spoof_checker_data;

static void spoof_checker_free(void* _this)
{
    icu_spoof_checker_data* this = _this;
    uspoof_close(this->checker);
}

static size_t spoof_checker_memsize(const void* _)
{
    return sizeof(icu_spoof_checker_data);
}

static const rb_data_type_t icu_spoof_checker_type = {
    "icu/spoof_checker",
    {NULL, spoof_checker_free, spoof_checker_memsize,},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE spoof_checker_alloc(VALUE self)
{
    icu_spoof_checker_type* this;
    return TypedData_Make_Struct(self, icu_spoof_checker_data, &icu_spoof_checker_type, this);
}

VALUE spoof_checker_initialize(VALUE self)
{
    GET_SPOOF_CHECKER(this);
    this->instance = self;
    this->checker = FALSE;

    UErrorCode status = U_ZERO_ERROR;
    this->checker = uspoof_open(&status);
    if (U_FAILURE(status)) {
        rb_raise(rb_eICU_Error, u_errorName(status));
    }

    return self;
}

void init_icu_spoof_checker(void)
{
    rb_cICU_SpoofChecker = rb_define_class_under(rb_mICU, "SpoofChecker", rb_cObject);
    rb_define_alloc_func(rb_cICU_SpoofChecker, spoof_checker_alloc);
    rb_define_method(rb_cICU_SpoofChecker, "initialize", spoof_checker_initialize, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level", spoof_checker_get_restriction_level, 0);
    rb_define_method(rb_cICU_SpoofChecker, "restriction_level=", spoof_checker_set_restriction_level, 1);
}

#undef GET_SPOOF_CHECKER

/* vim: set expandtab sws=4 sw=4: */
