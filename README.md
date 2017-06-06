ICU-next - A Unicode processing functions ruby gem - binding to ICU

Alpha stage. Proof of concept.

Ruby required 2.3.1.

[![Build Status](https://travis-ci.org/fantasticfears/icu4r-next.svg?branch=master)](https://travis-ci.org/fantasticfears/icu4r-next)

## Design

Right now, all arguments passed should be expected as Ruby `String`.

### Ruby API

Ruby API should be higher level and easily configurable.
The underlying encoding conversion should be transparent to the user.

### Encoding

Ruby has an [_Code Set Independent (CSI)_][ruby_m17n] model
for string implementation [because of the community][ruby_m17n_history].
Ruby's string should honour the `Encoding.default_internal`,
otherwise `__ENCODING__` (that includes newly created)
from the environment is used for encoding string.
The string holds only the byte array. A string may have unmatched encoding
or invalid bytes.

ICU uses UTF-16 internally. But there is also dedicate fast UTF-8 code path.
In "most" environment for Ruby community,
MRI will take UTF-8 as string's representation.
So the use of UTF-8 code path should be considered.
If possible, ICU should also be [compiled for `-DU_CHARSET_IS_UTF8=1`][icu_doc_utf_8].

Considered the fact above, the instances shall follow the encoding settings
to return the desired encoding generally.
The input string can be treated as UTF-8 can be used for its code path.
Otherwise, a conversion by ICU should be employed.
The output string should honor the encoding settings.
The conversion should be transparent to Ruby users.

Some details about MRI and encoding:

- `string.pack("U*")` actually returns Unicode Scalar value.
While `n*`rules don't know UTF16.
- macro `ENCODING_GET` retrieves an object's encoding index.
The encoding can be in object's `RBasic` or an instance variable
in that object depending on encoding's index.
- `rb_default_internal_encoding()` and `rb_enc_default_internal()`
returns the c struct encoding and ruby encoding object accordingly.
- `rb_locale_encindex()` gets the encoding index from the locale.

### Limitations

- It's a common problem C Ruby extension faced.

## Contributing

### TODO

- Glib2 gem?
- Support Ruby 2.2+. Rails 5 requires Ruby 2.2.
- support encoding other than UTF-8?
- stream support
- close ICU data in memory?

[ruby_m17n]: http://yokolet.blogspot.se/2009/07/design-and-implementation-of-ruby-m17n.html
[ruby_m17n_history]: http://yehudakatz.com/2010/05/05/ruby-1-9-encodings-a-primer-and-the-solution-for-rails/
[icu_doc_utf_8]: http://userguide.icu-project.org/strings/utf-8
