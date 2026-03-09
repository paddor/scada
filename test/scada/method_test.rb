require_relative "../spec_helper"

describe "Methods" do
  include ScadaHelper

  it "calls a simple method with string input/output" do
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

  it "calls a method with no output" do
    with_server do |server, port|
      called_with = nil
      server.add_method("ns=1;s=notify",
        display_name: "Notify",
        input: [{ name: "msg", type: :string }]) { |req| called_with = req[0]; nil }

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        result = client.call("ns=1;s=notify", "test").wait
        assert_nil result
        assert_equal "test", called_with
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "calls a method with numeric input/output" do
    with_server do |server, port|
      server.add_method("ns=1;s=double_it",
        display_name: "DoubleIt",
        input: [{ name: "x", type: :int32 }],
        output: [{ name: "result", type: :int32 }]) { |req| req[0] * 2 }

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        result = client.call("ns=1;s=double_it", 21).wait
        assert_equal 42, result
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
