require 'benchmark'
require 'icu'
require 'twitter_cldr'
require 'active_support'

STRING_RUN = 100000
PHRASES_RUN = 1000
TEXT_RUN = 100

# File is encoded as UTF-8
STRING = "Äffin"
PHRASES = File.read(File.expand_path('../normalization_phrases.txt', __FILE__))
TEXT = File.read(File.expand_path('../normalization_wikip.txt', __FILE__))

puts "", "Single string benchmark", ""

Benchmark.bmbm do |x|
  icu_normalizer = ICU::Normalizer.new(name = :nfc, mode = :decompose)

  x.report 'ICU normalizer nfd' do
    STRING_RUN.times do
      icu_normalizer.normalize(STRING)
    end
  end

  x.report 'Ruby stdlib normalizer' do
    STRING_RUN.times do
      STRING.unicode_normalize(:nfd)
    end
  end

  x.report 'twitter-cldr' do
    STRING_RUN.times do
      TwitterCldr::Normalization.normalize(STRING, using: :NFD)
    end
  end

  x.report 'Active Support' do
    STRING_RUN.times do
      ActiveSupport::Multibyte::Unicode.normalize(STRING, :d)
    end
  end
end

puts "", "Phrases benchmark", ""

Benchmark.bmbm do |x|
  icu_normalizer = ICU::Normalizer.new(name = :nfc, mode = :decompose)

  x.report 'ICU normalizer nfd' do
    PHRASES_RUN.times do
      icu_normalizer.normalize(PHRASES)
    end
  end

  x.report 'Ruby stdlib normalizer' do
    PHRASES_RUN.times do
      PHRASES.unicode_normalize(:nfd)
    end
  end

  x.report 'twitter-cldr' do
    PHRASES_RUN.times do
      TwitterCldr::Normalization.normalize(PHRASES, using: :NFD)
    end
  end

  x.report 'Active Support' do
    PHRASES_RUN.times do
      ActiveSupport::Multibyte::Unicode.normalize(PHRASES, :d)
    end
  end
end

puts "", "Article benchmark", ""

Benchmark.bmbm do |x|
  icu_normalizer = ICU::Normalizer.new(name = :nfc, mode = :decompose)

  x.report 'ICU normalizer nfd' do
    TEXT_RUN.times do
      icu_normalizer.normalize(TEXT)
    end
  end

  x.report 'Ruby stdlib normalizer' do
    TEXT_RUN.times do
      TEXT.unicode_normalize(:nfd)
    end
  end

  x.report 'twitter-cldr' do
    TEXT_RUN.times do
      TwitterCldr::Normalization.normalize(TEXT, using: :NFD)
    end
  end

  x.report 'Active Support' do
    TEXT_RUN.times do
      ActiveSupport::Multibyte::Unicode.normalize(TEXT, :d)
    end
  end
end
