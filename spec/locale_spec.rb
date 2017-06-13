require 'spec_helper'

describe ICU::Locale do
  describe '#avalable' do
    subject { ICU::Locale.available }

    it { is_expected.to be_an Array }
    it { is_expected.not_to be_empty }
    it { expect(subject.first).to be_a ICU::Locale }
  end

  describe '#iso_countries' do
    subject { ICU::Locale.iso_countries }

    it { is_expected.to be_an Array }
    it { is_expected.not_to be_empty }
    it { expect(subject.first).to be_a String }
  end

  describe '#default' do
    subject { ICU::Locale.default }

    let(:locale) do
      locales = ICU::Locale.available
      locales.delete(ICU::Locale.default)
      locales.respond_to?(:sample) ? locales.sample : locales.choice
    end

    it { is_expected.to be_a ICU::Locale }

    it 'can be assigned using Locale' do
      default_locale = locale
      expect(default_locale).to eq locale
    end

    it 'can be assigned using string' do
      string = locale.to_s

      ICU::Locale.default = string
      expect(subject.to_s).to eq string
      expect(subject).to eq ICU::Locale.new(string)
    end

    it 'can be assigned using symbol' do
      symbol = locale.to_s.to_sym

      ICU::Locale.default = symbol
      expect(ICU::Locale.default).to eq ICU::Locale.new(symbol)
    end
  end

  describe '.new' do
    it "raises when locale can't be encoded by ASCII" do
      expect { ICU::Locale.new("中文") }.to raise_error(Encoding::UndefinedConversionError)
    end
  end

  describe '.==' do
    let(:locale) { "en" }
    it 'returns true when the @id is exact the same and class matches' do
      expect(ICU::Locale.new(locale) == ICU::Locale.new("en")).to be_truthy
    end

    it 'returns false when the class is different' do
      class TmpLocale
        def id
          locale
        end
      end
      expect(ICU::Locale.new(locale) == TmpLocale.new).to be_falsey
    end
  end

  context 'with ICU locale ID' do
    describe '#for_language_tag' do
      it 'converts a language tag to a locale' do
        expect(ICU::Locale.for_language_tag('en-us')).to eq ICU::Locale.new('en_US')
        expect(ICU::Locale.for_language_tag('nan-Hant-tw')).to eq ICU::Locale.new('nan_Hant_TW')
      end
    end

    describe '.language_tag' do
      it 'returns a language tag for a locale' do
        expect(ICU::Locale.new('en_US').language_tag).to eq 'en-US'
        expect(ICU::Locale.new('zh_TW').language_tag).to eq 'zh-TW'
        expect(ICU::Locale.new('zh_Hans_CH_PINYIN').language_tag).to eq 'zh-Hans-CH-u-co-pinyin'
      end
    end

    describe '#for_lcid' do
      it 'converts an LCID to a locale' do
        expect(ICU::Locale.for_lcid(1033)).to eq ICU::Locale.new('en_US')
        expect(ICU::Locale.for_lcid(1036)).to eq ICU::Locale.new('fr_FR')
      end
    end

    describe '.lcid' do
      it 'returns an LCID for a locale' do
        expect(ICU::Locale.new('en_US').lcid).to eq 1033
        expect(ICU::Locale.new('es_US').lcid).to eq 21514
      end
    end

    describe '.display_country' do
      it 'returns the country' do
        expect(ICU::Locale.new('de_DE').display_country('en')).to eq 'Germany'
        expect(ICU::Locale.new('en_US').display_country('fr')).to eq 'États-Unis'
      end
    end

    describe '.display_language' do
      it 'returns the language' do
        expect(ICU::Locale.new('fr_FR').display_language('de')).to eq 'Französisch'
        expect(ICU::Locale.new('zh_CH').display_language('en')).to eq 'Chinese'
      end
    end

    describe '.display_name' do
      it 'returns the name' do
        expect(ICU::Locale.new('en_US').display_name('de')).to eq 'Englisch (Vereinigte Staaten)'
        expect(ICU::Locale.new('zh_CH').display_name('fr')).to eq 'chinois (Suisse)'
      end
    end

    describe '.display_script' do
      it 'returns the script' do
        expect(ICU::Locale.new('ja_Hira_JP').display_script('en')).to eq 'Hiragana'
        expect(ICU::Locale.new('ja_Hira_JP').display_script('ru')).to eq 'хирагана'
      end
    end

    describe '.display_variant' do
      it 'returns the variant' do
        expect(ICU::Locale.new('be_BY_TARASK').display_variant('de')).to eq 'Taraskievica-Orthographie'
        expect(ICU::Locale.new('zh_CH_POSIX').display_variant('en')).to eq 'Computer'
      end
    end

    context 'with default locale' do
      let(:default) { ICU::Locale.default }
      subject { ICU::Locale.new('de_DE') }

      it 'returns the country' do
        expect(subject.display_country).to eq subject.display_country(default)
      end

      it 'returns the language' do
        expect(subject.display_language).to eq subject.display_language(default)
      end

      it 'returns the name' do
        expect(subject.display_name).to eq subject.display_name(default)
      end

      it 'returns the script' do
        expect(subject.display_script).to eq subject.display_script(default)
      end

      it 'returns the variant' do
        expect(subject.display_variant).to eq subject.display_variant(default)
      end
    end
  end

  context 'formatting' do
    subject { ICU::Locale.new('de-de.utf8@collation = phonebook') }

    describe '.name' do
      it 'is formatted' do
        expect(subject.name).to eq 'de_DE.utf8@collation=phonebook'
      end
    end

    describe '.base_name' do
      it 'is formatted without keywords' do
        expect(subject.base_name).to eq 'de_DE.utf8'
      end
    end

    describe '.canonical_name' do
      it 'is formatted for ICU' do
        expect(subject.canonical_name).to eq 'de_DE@collation=phonebook'
      end
    end
  end

  describe '.parent' do
    it 'truncates a properly formatted locale, returning the "parent"' do
      expect(ICU::Locale.new('es-mx').parent).to eq ''
      expect(ICU::Locale.new('es_MX').parent).to eq 'es'
      expect(ICU::Locale.new('zh_Hans_CH_PINYIN').parent).to eq 'zh_Hans_CH'
    end
  end

  describe '.iso_country' do
    it 'returns the ISO 3166 alpha-3 country code' do
      expect(ICU::Locale.new('en_US').iso_country).to eq 'USA'
      expect(ICU::Locale.new('zh_CN').iso_country).to eq 'CHN'
    end
  end

  describe '.iso_language' do
    it 'returns the ISO 639 three-letter language code' do
      expect(ICU::Locale.new('en_US').iso_language).to eq 'eng'
      expect(ICU::Locale.new('zh_CN').iso_language).to eq 'zho'
    end
  end

  describe '.keywords' do
    context 'when improperly formatted' do
      let(:locale) { ICU::Locale.new('de_DE@euro') }

      it 'raises an error' do
        expect { locale.keywords }.to raise_error(ICU::Error)
      end
    end

    context 'when properly formatted' do
      let(:locale) { ICU::Locale.new('de_DE@currency=EUR') }

      it 'returns the list of keywords' do
        expect(locale.keywords).to eq ['currency']
      end
    end
  end

  describe '.keyword' do
    it 'can be read' do
      expect(ICU::Locale.new('en_US@calendar=chinese').keyword('calendar')).to eq 'chinese'
      expect(ICU::Locale.new('en_US@calendar=chinese').keyword(:calendar)).to eq 'chinese'
      expect(ICU::Locale.new('en_US@some=thing').keyword('missing')).to eq ''
    end
  end

  describe '.with_keyword' do
    it 'can be added' do
      expect(ICU::Locale.new('de_DE').with_keyword('currency', 'EUR')).to eq ICU::Locale.new('de_DE@currency=EUR')
      expect(ICU::Locale.new('de_DE').with_keyword(:currency, :EUR)).to eq ICU::Locale.new('de_DE@currency=EUR')
    end

    it 'can be removed' do
      expect(ICU::Locale.new('en_US@some=thing').with_keyword(:some, nil)).to eq ICU::Locale.new('en_US')
      expect(ICU::Locale.new('en_US@some=thing').with_keyword(:some, '')).to eq ICU::Locale.new('en_US')
    end
  end

  describe '.with_keywords' do
    it 'can be added using hash' do
      expect(ICU::Locale.new('fr').with_keywords(:a => :b, :c => :d)).to eq ICU::Locale.new('fr@a=b;c=d')
    end
  end

  describe '.character_orientation' do
    it 'returns the character orientation' do
      expect(ICU::Locale.new('ar').character_orientation).to eq :rtl
      expect(ICU::Locale.new('en').character_orientation).to eq :ltr
      expect(ICU::Locale.new('fa').character_orientation).to eq :rtl
    end
  end

  describe '.line_orientation' do
    it 'returns the line orientation' do
      expect(ICU::Locale.new('ar').line_orientation).to eq :ttb
      expect(ICU::Locale.new('en').line_orientation).to eq :ttb
      expect(ICU::Locale.new('fa').line_orientation).to eq :ttb
    end
  end

  context 'subtags' do
    subject { ICU::Locale.new('zh-hans-ch-pinyin') }

    describe '.country' do
      it 'returns the country code' do
        expect(subject.country).to eq 'CH'
      end
    end

    describe '.language' do
      it 'returns the language code' do
        expect(subject.language).to eq 'zh'
      end
    end

    describe '.script' do
      it 'returns the script code' do
        expect(subject.script).to eq 'Hans'
      end
    end

    describe '.variant' do
      it 'returns the variant code' do
        expect(subject.variant).to eq 'PINYIN'
      end
    end

    describe '.with_likely_subtags' do
      it 'adds likely subtags' do
        expect(ICU::Locale.new('en').with_likely_subtags).to eq ICU::Locale.new('en_Latn_US')
        expect(ICU::Locale.new('sr').with_likely_subtags).to eq ICU::Locale.new('sr_Cyrl_RS')
        expect(ICU::Locale.new('zh_TW').with_likely_subtags).to eq ICU::Locale.new('zh_Hant_TW')
      end
    end

    describe '.with_minimized_subtags' do
      it 'removes likely subtags' do
        expect(ICU::Locale.new('en_US').with_minimized_subtags).to eq ICU::Locale.new('en')
        expect(ICU::Locale.new('sr_RS').with_minimized_subtags).to eq ICU::Locale.new('sr')
        expect(ICU::Locale.new('zh_Hant_TW').with_minimized_subtags).to eq ICU::Locale.new('zh_TW')
      end
    end
  end
end
