module ICU
  class Locale
    attr_reader :id, :enc

    def ==(other)
      other.is_a?(self.class) && other.id == self.id
    end

    alias === ==
    alias to_s id

    def with_keywords(keywords)
      keywords.reduce(self) do |locale, (keyword, value)|
        # p locale, keyword, value
        locale.with_keyword(keyword, value)
      end
    end
  end
end