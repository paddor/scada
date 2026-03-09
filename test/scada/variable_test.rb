require_relative "../spec_helper"

describe "Variables" do
  include ScadaHelper

  it "reads initial double value" do
    with_server do |server, port|
      server.add_variable("ns=1;s=temp", type: :double, value: 22.0,
                          display_name: "Temperature")
      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=temp").wait
        assert_instance_of Scada::DataValue, dv
        assert_in_delta 22.0, dv.value, 0.01
        assert dv.status.good?
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "writes and reads back a double" do
    with_server do |server, port|
      server.add_variable("ns=1;s=setpoint", type: :double, value: 20.0,
                          display_name: "Setpoint")
      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        client.write("ns=1;s=setpoint", 35.0).wait
        dv = client.read("ns=1;s=setpoint").wait
        assert_in_delta 35.0, dv.value, 0.01
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "writes and reads a string variable" do
    with_server do |server, port|
      server.add_variable("ns=1;s=name", type: :string, value: "initial",
                          display_name: "Name")
      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        client.write("ns=1;s=name", "updated").wait
        dv = client.read("ns=1;s=name").wait
        assert_equal "updated", dv.value
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "writes and reads an integer variable" do
    with_server do |server, port|
      server.add_variable("ns=1;s=count", type: :int32, value: 0,
                          display_name: "Count")
      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        client.write("ns=1;s=count", 42).wait
        dv = client.read("ns=1;s=count").wait
        assert_equal 42, dv.value
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "writes and reads a boolean variable" do
    with_server do |server, port|
      server.add_variable("ns=1;s=flag", type: :boolean, value: false,
                          display_name: "Flag")
      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        client.write("ns=1;s=flag", true).wait
        dv = client.read("ns=1;s=flag").wait
        assert_equal true, dv.value
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
