require_relative "../spec_helper"

describe "Data Sources" do
  include ScadaHelper

  it "calls on_read callback when client reads" do
    with_server do |server, port|
      read_count = 0
      server.add_variable("ns=1;s=sensor", type: :double,
                          display_name: "Sensor",
                          on_read: -> { read_count += 1; 42.5 })

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=sensor").wait
        assert_in_delta 42.5, dv.value, 0.01
        assert_operator read_count, :>=, 1
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "calls on_write callback when client writes" do
    with_server do |server, port|
      written_value = nil
      server.add_variable("ns=1;s=actuator", type: :double,
                          display_name: "Actuator",
                          on_read: -> { 0.0 },
                          on_write: ->(old_dv, new_dv) { written_value = new_dv.value })

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        client.write("ns=1;s=actuator", 99.0).wait
        sleep 0.05
        assert_in_delta 99.0, written_value, 0.01
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
