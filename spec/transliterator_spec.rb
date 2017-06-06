require 'spec_helper'

def transliterator_for(*args)
  ICU::Transliterator.new(*args)
end

describe ICU::Transliterator do
  describe ".transliterate" do
    [
        ["Any-Hex", "abcde", "\\u0061\\u0062\\u0063\\u0064\\u0065"],
        ["Lower", "ABC", "abc"],
        ["Han-Latin", "雙屬性集合之空間分群演算法-應用於地理資料", "shuāng shǔ xìng jí hé zhī kōng jiān fēn qún yǎn suàn fǎ-yīng yòng yú de lǐ zī liào"],
        ["Devanagari-Latin", "दौलत", "daulata"]
    ].each do |id, input, output|
      it "should transliterate #{id}" do
        tl = transliterator_for(id)
        expect(tl.transliterate(input)).to eq output
      end
    end
  end

  describe '#available_ids' do
    subject { ICU::Transliterator }

    it "returns an array" do
      expect(subject.available_ids).to be_kind_of(Array)
    end

    it "returns an array with value" do
      expect(subject.available_ids).not_to be_empty
    end
  end

  describe '#transliterate' do
    subject { ICU::Transliterator }

    it "returns the exact transliteration" do
      expect(subject.transliterate("NFD; [:Nonspacing Mark:] Remove; NFC", "âêîôû")).to eq "aeiou"
    end
  end
end
