require_relative "../test_helper"

describe "Methods" do
  include ScadaHelper

  CALLED_WITH = [nil]

  @fixture = ScadaHelper.start_shared_server do |s|
    s.add_method('ns=1;s=greet',
      display_name: 'Greet',
      input: [{ name: 'name', type: :string }],
      output: [{ name: 'greeting', type: :string }]) do |req|
        "Hello, #{req[0]}!"
      end

    s.add_method('ns=1;s=notify',
      display_name: 'Notify',
      input: [{ name: 'msg', type: :string }]) do |req|
        CALLED_WITH[0] = req[0]; nil
      end

    s.add_method('ns=1;s=double_it',
      display_name: 'DoubleIt',
      input: [{ name: 'x', type: :int32 }],
      output: [{ name: 'result', type: :int32 }]) do |req|
        req[0] * 2
      end

    s.add_method('ns=1;s=get_future_time',
      display_name: 'GetFutureTime',
      input: [{ name: 'delay', type: :double }],
      output: [{ name: 'result', type: :datetime }]) do |req|
        delay = req[0]
        Async do
          sleep delay
          Time.now
        end
      end

    s.add_method('ns=1;s=async_fail',
      display_name: 'AsyncFail',
      output: [{ name: 'result', type: :string }]) do |req|
        Async(finished: false) do
          raise "boom"
        end
      end

    s.add_method('ns=1;s=async_fail_deferred',
      display_name: 'AsyncFailDeferred',
      output: [{ name: 'result', type: :string }]) do |req|
        Async(finished: false) do |t|
          t.yield
          raise "boom"
        end
      end
  end

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

  it "calls a method that completes asynchronously" do
    with_client(fixture) do |client|
      delay = 0.1
      t0 = Time.now
      result = client.call('ns=1;s=get_future_time', delay).wait
      elapsed = result.to_f - t0.to_f

      assert_in_delta delay, elapsed, 0.1
    end
  end

  it "returns an error when async method raises immediately" do
    with_client(fixture) do |client|
      err = assert_raises(Scada::Error) do
        client.call('ns=1;s=async_fail').wait
      end
      assert_match(/BadInternalError|0x80020000/i, err.message)
    end
  end

  it "returns an error when deferred async method raises" do
    with_client(fixture) do |client|
      err = assert_raises(Scada::Error) do
        client.call('ns=1;s=async_fail_deferred').wait
      end
      assert_match(/BadInternalError|0x80020000/i, err.message)
    end
  end
end
