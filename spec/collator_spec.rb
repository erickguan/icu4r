require "spec_helper"

describe ICU::Collator do
  let(:loc) { "nb" }
  subject { ICU::Collator.new(loc) }

  describe '.sort' do
    it "should sort an array of strings" do
      expect(subject.sort(%w[å ø æ])).to eq %w[æ ø å]
    end
  end

  describe '.compare' do
    it "should compare two strings" do
      expect(subject.compare("blåbærsyltetøy", "blah")).to eq 1
      expect(subject.compare("blåbærsyltetøy".encode("UTF-16"), "blah")).to eq 1
      expect(subject.compare("blåbærsyltetøy", "blah")).to eq 1
      expect(subject.compare("blah", "blah")).to eq 0
      expect(subject.compare("blah".encode("UTF-16"), "blah".encode("UTF-32"))).to eq 0
      expect(subject.compare("ba", "bl")).to eq -1
    end
  end

  describe '.locale' do
    subject { ICU::Collator.new("en_US_CALIFORNIA") }

    it 'returns the valid locale of the collator' do
      expect(subject.locale).to eq "en_US"
    end

    it 'returns the actual locale of the collator' do
      expect(subject.locale(:actual)).to eq "root"
    end
  end

  describe '.greater?' do
    it "returns true when the former is greater" do
      expect(subject.greater?("z", "a")).to be_truthy
      expect(subject.greater?("a", "z")).to be_falsey
    end
  end

  describe '.greater_or_equal?' do
    it "returns true when the former is greater or equal" do
      expect(subject.greater_or_equal?("z", "a")).to be_truthy
      expect(subject.greater_or_equal?("z", "z")).to be_truthy
      expect(subject.greater_or_equal?("a", "z")).to be_falsey
    end
  end

  describe '.equal?' do
    it "returns true when the former is equal" do
      expect(subject.equal?("a", "a")).to be_truthy
      expect(subject.equal?("a", "b")).to be_falsey
    end
  end

  describe '.rules' do
    it "should return rules" do
      expect(subject.rules).not_to be_empty
      # ö sorts before Ö
      expect(subject.rules.include?('ö<<<Ö')).to be_truthy
    end
  end

  describe '#sort' do
    subject { ICU::Collator }

    it "sorts the array of strings" do
      expect(subject.sort(loc, %w[å ø æ])).to eq %w[æ ø å]
    end
  end
end