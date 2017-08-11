require 'spec_helper'

describe ICU::SpoofChecker do
  let(:checker) { ICU::SpoofChecker.new }
  describe 'get and set checks' do
    it 'is successful' do
      checker.checks = ICU::SpoofChecker.available_checks[:char_limit] | ICU::SpoofChecker.available_checks[:mixed_numbers]
      expect(checker.checks).to eq ICU::SpoofChecker.available_checks[:char_limit] | ICU::SpoofChecker.available_checks[:mixed_numbers]
      checker.checks = ICU::SpoofChecker.available_checks[:all_checks] & ~ICU::SpoofChecker.available_checks[:confusable]
      expect(checker.checks).to eq ICU::SpoofChecker.available_checks[:all_checks] & ~ICU::SpoofChecker.available_checks[:confusable]
    end
  end

  describe 'get and set restriction_level' do
    it 'is successful' do
      checker.restriction_level = ICU::SpoofChecker.available_restriction_levels[:highly_restrictive]
      expect(checker.restriction_level).to eq ICU::SpoofChecker.available_restriction_levels[:highly_restrictive]
    end
  end

  describe '.confusable?' do
    shared_examples "confusable example" do |encodings|
      encodings.each do |encoding|
        it "can examine the confusable" do
          expect(checker.confusable?("lscopecC鬼obƅa", "1scopecC⿁оbьа")).to be > 0
          expect(checker.confusable?("lscopecC鬼obƅa".encode(encoding), "1scopecC⿁оbьа")).to be > 0
          expect(checker.confusable?("lscopecC鬼obƅa".encode(encoding), "1scopecC⿁оbьа".encode(encoding))).to be > 0
        end
      end
    end

    shared_examples "normal example" do |encodings|
      encodings.each do |encoding|
        it "returns 0 when it's not confusable" do
          expect(checker.confusable?("abc", "def")).to eq 0
          expect(checker.confusable?("abc".encode(encoding), "def")).to eq 0
          expect(checker.confusable?("abc".encode(encoding), "def".encode(encoding))).to eq 0
        end
      end
    end

    it_should_behave_like "confusable example", %w(UTF-8 UTF-16 UTF-32)
    it_should_behave_like "normal example", %w(UTF-8 UTF-16 UTF-32)
  end

  describe '.check' do
    it 'returns a integer contains about check result' do
      expect(checker.check('sсcs')).to eq ICU::SpoofChecker.available_checks[:restriction_level]
    end
  end

  describe '.get_skeleton' do
    it 'can gets the skeleton representation' do
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ")).not_to be_empty
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == checker.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ".encode("UTF-16")) == checker.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ".encode("UTF-16")) == checker.get_skeleton("ρ⍺у𝓅𝒂ן".encode("UTF-16"))).to be_truthy
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == checker.get_skeleton("paypal")).to be_truthy
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == checker.get_skeleton("ρ⍺у𝓅𝒂ן")).to be_truthy
      expect(checker.get_skeleton("𝔭𝒶ỿ𝕡𝕒ℓ") == checker.get_skeleton("paypal")).to be_truthy
    end
  end
end
