require_relative "../spec_helper"

describe Scada::Client do
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

  describe "connect" do
    it "connects to a running server" do
      with_client(fixture) { |_client| } # connect succeeds
    end
  end

  describe "#read" do
    it "reads a single variable value" do
      with_client(fixture) do |client|
        dv = client.read('ns=1;s=temp').wait
        assert_instance_of Scada::DataValue, dv
        assert_in_delta 22.5, dv.value, 0.01
        assert dv.status.good?
      end
    end

    it "reads display_name attribute" do
      with_client(fixture) do |client|
        name = client.read('ns=1;s=temp', attribute: :display_name).wait
        assert_equal 'Temperature', name
      end
    end
  end

  describe "#write" do
    it "writes a value and reads it back" do
      with_client(fixture) do |client|
        client.write('ns=1;s=setpoint', 42.0).wait
        dv = client.read('ns=1;s=setpoint').wait
        assert_in_delta 42.0, dv.value, 0.01
      end
    end
  end

  describe "#call" do
    it "calls a method and gets the result" do
      with_client(fixture) do |client|
        result = client.call('ns=1;s=greet', 'World').wait
        assert_equal 'Hello, World!', result
      end
    end
  end

  describe "error handling" do
    it "raises on reading nonexistent node" do
      with_client(fixture) do |client|
        assert_raises(Scada::Error) do
          client.read('ns=1;s=nonexistent').wait
        end
      end
    end
  end
end
