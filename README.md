ICU - A Unicode processing functions ruby gem - binding to ICU

Beta stage.

Ruby required 2.3.1.

[![Build Status](https://travis-ci.org/fantasticfears/icu4r.svg?branch=master)](https://travis-ci.org/fantasticfears/icu4r)

## Usage

If you use OS X,

```
brew install icu4c
gem install icu -- --use-system-libraries
```

else,

```
gem install icu
```

For usage:

```
require 'icu'
```

## Design

Almost all arguments passed should be expected as Ruby `String` with various encodings.
Sometimes, symbol is also allowed. More specifically, `ICU::Locale` accepts only ASCII-compatible string.

### Ruby API

Ruby API should be higher level and easily configurable.
The underlying encoding conversion should be transparent to the user.

### Encoding

Ruby has an [_Code Set Independent (CSI)_][ruby_m17n] model
for string implementation [because of its community][ruby_m17n_history].
Ruby's string should honour the `Encoding.default_internal`,
otherwise `__ENCODING__` (including newly created string)
from the environment is used for encoding string.
The string holds only the byte array. A string may have unmatched encoding
or invalid bytes.

ICU uses UTF-16 internally. But there is also dedicate fast UTF-8 code path.
In most environment for Ruby community,
MRI will take UTF-8 as string's representation.
So the use of UTF-8 code path should be considered.
If possible and matches with the MRI encoding,
ICU should also be [compiled for `-DU_CHARSET_IS_UTF8=1`][icu_doc_utf_8].

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

## Contributing

Feel free to fork and submit a pull request.

### TODO

- Support Ruby 2.2+. Rails 5 requires Ruby 2.2.2.
- Merge ffi-icu. [This branch](https://github.com/fantasticfears/icu4r/tree/feature-number-formatter) can be a start
- Merge some resources from [this branch (old `icu` gem)](https://github.com/fantasticfears/icu4r/tree/old-icu4r).
- port time/number_formatting module from [ffi-icu](https://github.com/fantasticfears/ffi-icu).
- binary distribution of ICU & system library support
- documentation

[ruby_m17n]: http://yokolet.blogspot.se/2009/07/design-and-implementation-of-ruby-m17n.html
[ruby_m17n_history]: http://yehudakatz.com/2010/05/05/ruby-1-9-encodings-a-primer-and-the-solution-for-rails/
[icu_doc_utf_8]: http://userguide.icu-project.org/strings/utf-8
