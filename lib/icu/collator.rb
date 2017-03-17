module ICU
  class Collator
    def sort(strings)
      strings.sort { |a, b| compare(a, b) }
    end
  end
end