require_relative "../spec_helper"

describe "Methods" do
  include ScadaHelper

  CALLED_WITH = [nil]

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_method('ns=1;s=greet',
      display_name: 'Greet',
      input: [{ name: 'name', type: :string }],
      output: [{ name: 'greeting', type: :string }]) { |req|
        "Hello, #{req[0]}!"
      }

    s.add_method('ns=1;s=notify',
      display_name: 'Notify',
      input: [{ name: 'msg', type: :string }]) { |req|
        CALLED_WITH[0] = req[0]; nil
      }

    s.add_method('ns=1;s=double_it',
      display_name: 'DoubleIt',
      input: [{ name: 'x', type: :int32 }],
      output: [{ name: 'result', type: :int32 }]) { |req|
        req[0] * 2
      }
  }

  it "calls a simple method with string input/output" do
    with_client(fixture) do |client|
      result = client.call('ns=1;s=greet', 'World').wait
      assert_equal 'Hello, World!', result
    end
  end

  it "calls a method with no output" do
    with_client(fixture) do |client|
      CALLED_WITH[0] = nil
      result = client.call('ns=1;s=notify', 'test').wait
      assert_nil result
      assert_equal 'test', CALLED_WITH[0]
    end
  end

  it "calls a method with numeric input/output" do
    with_client(fixture) do |client|
      result = client.call('ns=1;s=double_it', 21).wait
      assert_equal 42, result
    end
  end
end
