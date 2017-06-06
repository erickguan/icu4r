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
      subject.restriction_level = ICU::SpoofChecker::RestrictionLevel::HIGHLY_RESTRICTIVE
      expect(subject.restriction_level).to eq ICU::SpoofChecker::RestrictionLevel::HIGHLY_RESTRICTIVE
    end
  end

  describe '.confusable?' do
    shared_examples "confusable example" do |encodings|
      encodings.each do |encoding|
        it "can examine the confusable" do
          expect(subject.confusable?("lscopecC鬼obƅa", "1scopecC⿁оbьа")).to be > 0
          expect(subject.confusable?("lscopecC鬼obƅa".encode(encoding), "1scopecC⿁оbьа")).to be > 0
          expect(subject.confusable?("lscopecC鬼obƅa".encode(encoding), "1scopecC⿁оbьа".encode(encoding))).to be > 0
        end
      end
    end

    shared_examples "normal example" do |encodings|
      encodings.each do |encoding|
        it "returns 0 when it's not confusable" do
          expect(subject.confusable?("abc", "def")).to eq 0
          expect(subject.confusable?("abc".encode(encoding), "def")).to eq 0
          expect(subject.confusable?("abc".encode(encoding), "def".encode(encoding))).to eq 0
        end
      end
    end

    it_should_behave_like "confusable example", %w(UTF-8 UTF-16 UTF-32)
    it_should_behave_like "normal example", %w(UTF-8 UTF-16 UTF-32)
  end

  describe '.get_skeleton' do
    it 'can gets the skeleton representation' do
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ")).not_to be_empty
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ".encode("UTF-16")) == subject.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ".encode("UTF-16")) == subject.get_skeleton("ρ⍺у𝓅𝒂ן".encode("UTF-16"))).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("paypal")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(subject.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == subject.get_skeleton("paypal")).to be_truthy
    end
  end
end
