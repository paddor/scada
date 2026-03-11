require_relative "../spec_helper"

describe Scada::Client do
  include ScadaHelper

  describe "transparent namespace mapping" do
    @fixture = ScadaHelper.start_shared_server { |s|
      ns_a = s.add_namespace('urn:scada:test:alpha')
      ns_b = s.add_namespace('urn:scada:test:beta')
      s.add_variable("ns=#{ns_a};s=pressure", type: :double,
                      value: 101.3, display_name: 'Pressure')
      s.add_variable("ns=#{ns_a};s=a_var", type: :int32, value: 1,
                      display_name: 'Alpha Var')
      s.add_variable("ns=#{ns_b};s=b_var", type: :int32, value: 2,
                      display_name: 'Beta Var')
    }

    it "resolves a server namespace URI to local index after connect" do
      with_client(fixture) do |client|
        local_idx = client.get_namespace_index('urn:scada:test:alpha')
        assert_operator local_idx, :>=, 2

        dv = client.read("ns=#{local_idx};s=pressure").wait
        assert_instance_of Scada::DataValue, dv
        assert_in_delta 101.3, dv.value, 0.01
      end
    end

    it "maps multiple custom namespaces" do
      with_client(fixture) do |client|
        idx_a = client.get_namespace_index('urn:scada:test:alpha')
        idx_b = client.get_namespace_index('urn:scada:test:beta')
        refute_equal idx_a, idx_b

        dv_a = client.read("ns=#{idx_a};s=a_var").wait
        dv_b = client.read("ns=#{idx_b};s=b_var").wait
        assert_equal 1, dv_a.value
        assert_equal 2, dv_b.value
      end
    end
  end
end
