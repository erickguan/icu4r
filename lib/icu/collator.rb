module ICU
  class Collator
    def self.sort(locale, strings)
      self.new(locale)
          .sort(strings)
    end

    def sort(strings)
      strings.sort { |a, b| compare(a, b) }
    end

    def greater?(str_a, str_b)
      compare(str_a, str_b) > 0
    end

    def greater_or_equal?(str_a, str_b)
      compare(str_a, str_b) >= 0
    end

    def equal?(str_a, str_b)
      compare(str_a, str_b).zero?
    end
  end
end