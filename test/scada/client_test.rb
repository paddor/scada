require_relative "../spec_helper"

describe Scada::Client do
  include ScadaHelper

  describe "connect" do
    it "connects to a running server" do
      with_server do |server, port|
        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
        ensure
          server_task&.stop
        end
      end
    end
  end

  describe "#read" do
    it "reads a single variable value" do
      with_server do |server, port|
        server.add_variable("ns=1;s=temp", type: :double, value: 22.5,
                            display_name: "Temperature")
        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          dv = client.read("ns=1;s=temp").wait
          assert_instance_of Scada::DataValue, dv
          assert_in_delta 22.5, dv.value, 0.01
          assert dv.status.good?
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end

    it "reads display_name attribute" do
      with_server do |server, port|
        server.add_variable("ns=1;s=temp", type: :double, value: 22.5,
                            display_name: "Temperature")
        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          name = client.read("ns=1;s=temp", attribute: :display_name).wait
          assert_equal "Temperature", name
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end
  end

  describe "#write" do
    it "writes a value and reads it back" do
      with_server do |server, port|
        server.add_variable("ns=1;s=setpoint", type: :double, value: 20.0,
                            display_name: "Setpoint")
        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          client.write("ns=1;s=setpoint", 42.0).wait
          dv = client.read("ns=1;s=setpoint").wait
          assert_in_delta 42.0, dv.value, 0.01
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end
  end

  describe "#call" do
    it "calls a method and gets the result" do
      with_server do |server, port|
        server.add_method("ns=1;s=greet",
          display_name: "Greet",
          input: [{ name: "name", type: :string }],
          output: [{ name: "greeting", type: :string }]) { |req| "Hello, #{req[0]}!" }

        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          result = client.call("ns=1;s=greet", "World").wait
          assert_equal "Hello, World!", result
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end
  end

  describe "error handling" do
    it "raises on reading nonexistent node" do
      with_server do |server, port|
        Sync do |task|
          server_task = task.async { server.run }
          client = new_client("opc.tcp://localhost:#{port}")
          client.connect
          client_task = task.async { client.run }

          assert_raises(Scada::Error::Error) do
            client.read("ns=1;s=nonexistent").wait
          end
        ensure
          client_task&.stop
          server_task&.stop
        end
      end
    end
  end
end
