require 'spec_helper'

describe ICU::SpoofChecker do
  subject { ICU::SpoofChecker.new }

  describe 'get and set checks' do
    it 'is successful' do
      subject.checks = ICU::SpoofChecker::Checks::CHAR_LIMIT | ICU::SpoofChecker::Checks::MIXED_NUMBERS
      expect(subject.checks).to eq ICU::SpoofChecker::Checks::CHAR_LIMIT | ICU::SpoofChecker::Checks::MIXED_NUMBERS
      subject.checks = ICU::SpoofChecker::Checks::ALL_CHECKS & ~ICU::SpoofChecker::Checks::CONFUSABLE
      expect(subject.checks).to eq ICU::SpoofChecker::Checks::ALL_CHECKS & ~ICU::SpoofChecker::Checks::CONFUSABLE
    end
  end

  describe 'get and set restriction_level' do
    it 'is successful' do
      subject.restriction_level = ICU::SpoofChecker::HIGHLY_RESTRICTIVE
      expect(subject.restriction_level).to eq ICU::SpoofChecker::HIGHLY_RESTRICTIVE
    end
  end

  describe '.confusable?' do
    it 'can examine the confusable' do
      expect(subject.confusable?("lscopecC鬼obƅa", "1scopecC⿁оbьа")).to be_truthy
    end

    it "returns false when it's not confusable" do
      expect(subject.confusable?("abc", "def")).to eq 0
    end
  end

  describe '.get_skeleton' do
    it 'can gets the skeleton representation' do
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("paypal")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("paypal")).to be_truthy
    end
  end
end
