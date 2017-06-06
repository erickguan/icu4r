module ICU
  class Transliterator
    def self.transliterate(id, str, rules = nil, direction = nil)
      self.new(id, rules, direction)
          .transliterate(str)
    end
  end
end