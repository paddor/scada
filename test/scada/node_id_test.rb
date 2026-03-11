require_relative "../test_helper"

describe Scada::NodeId do
  describe ".parse" do
    it "parses numeric node IDs" do
      nid = Scada::NodeId.parse("ns=1;i=42")
      assert_equal 1, nid.namespace
      assert_equal "ns=1;i=42", nid.to_s
    end

    it "parses string node IDs" do
      nid = Scada::NodeId.parse("ns=2;s=temperature")
      assert_equal 2, nid.namespace
      assert_equal "ns=2;s=temperature", nid.to_s
    end

    it "parses ns=0 numeric node IDs" do
      nid = Scada::NodeId.parse("i=85")
      assert_equal 0, nid.namespace
      assert_equal "i=85", nid.to_s
    end

    it "raises on invalid input" do
      assert_raises(ArgumentError) { Scada::NodeId.parse("garbage") }
    end
  end

  describe "#==" do
    it "considers equal node IDs equal" do
      a = Scada::NodeId.parse("ns=1;i=42")
      b = Scada::NodeId.parse("ns=1;i=42")
      assert_equal a, b
    end

    it "considers different node IDs not equal" do
      a = Scada::NodeId.parse("ns=1;i=42")
      b = Scada::NodeId.parse("ns=1;i=43")
      refute_equal a, b
    end

    it "considers different namespaces not equal" do
      a = Scada::NodeId.parse("ns=1;i=42")
      b = Scada::NodeId.parse("ns=2;i=42")
      refute_equal a, b
    end
  end

  describe "#hash" do
    it "equal node IDs have equal hashes" do
      a = Scada::NodeId.parse("ns=1;i=42")
      b = Scada::NodeId.parse("ns=1;i=42")
      assert_equal a.hash, b.hash
    end

    it "works as hash key" do
      nid = Scada::NodeId.parse("ns=1;i=42")
      h = { nid => "value" }
      assert_equal "value", h[Scada::NodeId.parse("ns=1;i=42")]
    end
  end

  describe "#inspect" do
    it "returns readable representation" do
      nid = Scada::NodeId.parse("ns=1;i=42")
      assert_includes nid.inspect, "Scada::NodeId"
      assert_includes nid.inspect, "ns=1;i=42"
    end
  end
end
