require 'spec_helper'

describe ICU::Normalizer do
  context 'nfd decomposed' do
    subject { ICU::Normalizer.new(:nfc, :decompose) }

    describe '.normalize' do
      shared_examples "normalization example" do |encodings|
        encodings.each do |encoding|
          it "should normalize a string - decomposed" do
            expect(subject.normalize("Å".encode(encoding)).unpack("U*")).to eq [65, 778]
          end
        end
      end

      it_should_behave_like "normalization example", %w(UTF-8 UTF-16 UTF-32 ISO-8859-1)
    end
  end

  context 'nfc composed' do
    subject { ICU::Normalizer.new(:nfc, :compose) }

    describe '.normalize' do
      shared_examples "normalization example" do |encodings|
        encodings.each do |encoding|
          it "should normalize a string - composed" do
            expect(subject.normalize("Å".encode(encoding)).unpack("U*")).to eq [197]
          end
        end
      end

      it_should_behave_like "normalization example", %w(UTF-8 UTF-16 UTF-32 ISO-8859-1)
    end
  end
end
