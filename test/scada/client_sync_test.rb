require_relative "../test_helper"

describe "Scada::Client (sync mode)" do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=temp', type: :double, value: 22.5,
                    display_name: 'Temperature')
    s.add_variable('ns=1;s=setpoint', type: :double, value: 20.0,
                    display_name: 'Setpoint')
    s.add_method('ns=1;s=greet',
      display_name: 'Greet',
      input: [{ name: 'name', type: :string }],
      output: [{ name: 'greeting', type: :string }]) { |req|
        "Hello, #{req[0]}!"
      }
  }

  describe "#connect" do
    it "connects synchronously without Async context" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      client.close
    end
  end

  describe "#read" do
    it "reads a single variable synchronously" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      dv = client.read('ns=1;s=temp')
      assert_instance_of Scada::DataValue, dv
      assert_in_delta 22.5, dv.value, 0.01
      assert dv.status.good?
    ensure
      client&.close
    end

    it "reads display_name attribute synchronously" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      name = client.read('ns=1;s=temp', attribute: :display_name)
      assert_equal 'Temperature', name
    ensure
      client&.close
    end

    it "returns an array for multi-read" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      results = client.read('ns=1;s=temp', 'ns=1;s=setpoint')
      assert_instance_of Array, results
      assert_equal 2, results.length
      assert_in_delta 22.5, results[0].value, 0.01
    ensure
      client&.close
    end
  end

  describe "#write" do
    it "writes a value and reads it back synchronously" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      client.write('ns=1;s=setpoint', 99.0)
      dv = client.read('ns=1;s=setpoint')
      assert_in_delta 99.0, dv.value, 0.01
    ensure
      client&.close
    end
  end

  describe "#call" do
    it "calls a method synchronously" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      result = client.call('ns=1;s=greet', 'World')
      assert_equal 'Hello, World!', result
    ensure
      client&.close
    end
  end

  describe "error handling" do
    it "raises on reading nonexistent node" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      assert_raises(Scada::Error) do
        client.read('ns=1;s=nonexistent')
      end
    ensure
      client&.close
    end
  end

  describe "#run" do
    it "raises outside Async context" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      err = assert_raises(Scada::Error) { client.run }
      assert_match(/Async context/, err.message)
    ensure
      client&.close
    end
  end

  describe "#subscribe" do
    it "raises outside Async context" do
      client = new_client("opc.tcp://localhost:#{fixture.port}")
      client.connect
      err = assert_raises(Scada::Error) { client.subscribe }
      assert_match(/Async context/, err.message)
    ensure
      client&.close
    end
  end
end
