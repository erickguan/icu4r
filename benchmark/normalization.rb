require 'benchmark'
require 'icu'
require 'twitter_cldr'

TIMES = 100000

# File is encoded as UTF-8
CONTENT = File.read(File.expand_path('../normalization.txt', __FILE__))[0..500]

Benchmark.bmbm do |x|
  icu_normalizer = ICU::Normalizer.new(name = :nfc, mode = :decompose)

  x.report 'ICU normalizer nfd' do
    TIMES.times do
      icu_normalizer.normalize(CONTENT)
    end
  end

  CONTENT.unicode_normalize(:nfd)
  x.report 'Ruby stdlib normalizer' do
    TIMES.times do
      CONTENT.unicode_normalize(:nfd)
    end
  end

  TwitterCldr::Normalization.normalize(CONTENT, using: :NFD)
  x.report 'twitter-cldr' do
    TIMES.times do
      TwitterCldr::Normalization.normalize(CONTENT, using: :NFD)
    end
  end
end