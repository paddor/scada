require_relative "../spec_helper"

describe Scada::Client do
  include ScadaHelper

  describe "transparent namespace mapping" do
    it "resolves a server namespace URI to local index after connect" do
      with_server do |server, port|
        ns_idx = server.add_namespace("urn:scada:test:ns")
        server.add_variable("ns=#{ns_idx};s=pressure", type: :double, value: 101.3,
                            display_name: "Pressure")

        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          # v1.5: local namespace lookup from the client's mapping table
          # (auto-populated during connect)
          local_idx = client.get_namespace_index("urn:scada:test:ns")
          assert_operator local_idx, :>=, 2

          dv = client.read("ns=#{local_idx};s=pressure").wait
          assert_instance_of Scada::DataValue, dv
          assert_in_delta 101.3, dv.value, 0.01
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end

    it "maps multiple custom namespaces" do
      with_server do |server, port|
        ns_a = server.add_namespace("urn:scada:test:alpha")
        ns_b = server.add_namespace("urn:scada:test:beta")
        server.add_variable("ns=#{ns_a};s=a_var", type: :int32, value: 1,
                            display_name: "Alpha Var")
        server.add_variable("ns=#{ns_b};s=b_var", type: :int32, value: 2,
                            display_name: "Beta Var")

        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          idx_a = client.get_namespace_index("urn:scada:test:alpha")
          idx_b = client.get_namespace_index("urn:scada:test:beta")
          refute_equal idx_a, idx_b

          dv_a = client.read("ns=#{idx_a};s=a_var").wait
          dv_b = client.read("ns=#{idx_b};s=b_var").wait
          assert_equal 1, dv_a.value
          assert_equal 2, dv_b.value
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end
  end
end
