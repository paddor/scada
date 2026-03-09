require_relative "../spec_helper"

describe "Subscriptions" do
  include ScadaHelper

  it "fires data change callback on value change" do
    with_server do |server, port|
      server.add_variable("ns=1;s=temp", type: :double, value: 20.0,
                          display_name: "Temperature")

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        received = []
        sub = client.subscribe(publish_interval: 0.05)
        sub.monitor_data_changes("ns=1;s=temp") { |dv| received << dv }

        # Wait for initial value notification
        sleep 0.3

        # Write new value
        client.write("ns=1;s=temp", 30.0).wait
        sleep 0.3

        assert_operator received.size, :>=, 1
        assert_instance_of Scada::DataValue, received.last
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "monitor is an alias for monitor_data_changes" do
    with_server do |server, port|
      server.add_variable("ns=1;s=temp", type: :double, value: 20.0,
                          display_name: "Temperature")

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        received = []
        sub = client.subscribe(publish_interval: 0.05)
        sub.monitor("ns=1;s=temp") { |dv| received << dv }

        sleep 0.3
        assert_operator received.size, :>=, 1
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
