# encoding: UTF-8

require 'spec_helper'

describe ICU::CharsetDetector do
  describe '.detect' do
    it "should recognize UTF-8" do
      m = subject.detect("æåø")
      expect(m.name).to eq "UTF-8"
      expect(m.language).to be_kind_of(String)
    end

    it "should support null bytes" do
      # Create a utf-16 string and then force it to binary (ascii) to mimic data from net/http
      string = "foo".encode("UTF-16").force_encoding("binary")
      m = subject.detect(string)
      expect(m.name).to eq "UTF-16BE"
      expect(m.language).to be_kind_of(String)
    end
  end

  describe '.detect_all' do
    it "should detect several matching encodings" do
      expect(subject.detect_all("foo bar")).to be_instance_of(Array)
    end
  end

  describe "input filter" do
    it "should disable / enable the input filter" do
      subject.input_filter = false
      expect(subject.input_filter_enabled?).to be_falsey
      expect(subject.input_filter).to be_falsey
      subject.input_filter = true
      expect(subject.input_filter_enabled?).to be_truthy
      expect(subject.input_filter).to be_truthy
    end
  end

  describe '.detectable_charsets' do
    it "returns an array of detectable charsets" do
      cs = subject.detectable_charsets
      expect(cs).to be_kind_of(Array)
      expect(cs).not_to be_empty
      expect(cs.first).to be_kind_of(String)
    end
  end
end
