require 'spec_helper'

describe ICU::Normalizer do
  context 'nfd decomposed' do
    subject { ICU::Normalizer.new(:nfc, :decompose) }

    describe '.normalize' do
      it "should normalize a string - decomposed" do
        expect(subject.normalize("Å").unpack("U*")).to eq [65, 778]
      end
    end
  end

  context 'nfc composed' do
    subject { ICU::Normalizer.new(:nfc, :compose) }

    describe '.normalize' do
      it "should normalize a string - composed" do
        expect(subject.normalize("Å").unpack("U*")).to eq [197]
      end
    end
  end
end
