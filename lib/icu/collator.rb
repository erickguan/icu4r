module ICU
  class Collator
    def sort(strings)
      strings.sort { |a, b| compare(a, b) }
    end

    def greater?(str_a, str_b)
      compare(str_a, str_b) > 0
    end

    def greater_or_equal?(str_a, str_b)
      !(compare(str_a, str_b) < 0)
    end

    def equal?(str_a, str_b)
      compare(str_a, str_b) == 0
    end
  end
end