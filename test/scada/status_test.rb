require_relative "../spec_helper"

describe Scada::Status do
  describe "#good?" do
    it "returns true for GOOD status" do
      status = Scada::Status.new(code: 0x00000000)
      assert status.good?
    end

    it "returns false for BAD status" do
      status = Scada::Status.new(code: 0x80000000)
      refute status.good?
    end
  end

  describe "#bad?" do
    it "returns true for BAD status" do
      status = Scada::Status.new(code: 0x80010000)
      assert status.bad?
    end

    it "returns false for GOOD status" do
      status = Scada::Status.new(code: 0x00000000)
      refute status.bad?
    end
  end

  describe "#uncertain?" do
    it "returns true for uncertain status" do
      status = Scada::Status.new(code: 0x40000000)
      assert status.uncertain?
    end

    it "returns false for good status" do
      status = Scada::Status.new(code: 0x00000000)
      refute status.uncertain?
    end

    it "returns false for bad status" do
      status = Scada::Status.new(code: 0x80000000)
      refute status.uncertain?
    end
  end

  describe "#to_s" do
    it "returns hex representation" do
      status = Scada::Status.new(code: 0x80010000)
      assert_equal "0x80010000", status.to_s
    end
  end

  describe "#inspect" do
    it "includes class name and hex code" do
      status = Scada::Status.new(code: 0x00000000)
      assert_includes status.inspect, "Scada::Status"
      assert_includes status.inspect, "0x00000000"
    end
  end
end
