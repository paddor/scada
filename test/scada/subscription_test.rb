require_relative "../test_helper"

describe "Subscriptions" do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=temp', type: :double, value: 20.0,
                    display_name: 'Temperature')
  }

  it "fires data change callback on value change" do
    with_client(fixture) do |client|
      received = []
      sub = client.subscribe(publish_interval: 0.01)
      sub.monitor_data_changes('ns=1;s=temp') { |dv| received << dv }

      # Wait for initial value notification
      deadline = Async::Clock.now + 2.0
      sleep 0.01 until received.size >= 1 ||
                           Async::Clock.now > deadline

      # Write new value
      client.write('ns=1;s=temp', 30.0).wait
      initial_count = received.size
      deadline = Async::Clock.now + 2.0
      sleep 0.01 until received.size > initial_count ||
                           Async::Clock.now > deadline

      assert_operator received.size, :>=, 1
      assert_instance_of Scada::DataValue, received.last
    end
  end

  it "monitor is an alias for monitor_data_changes" do
    with_client(fixture) do |client|
      received = []
      sub = client.subscribe(publish_interval: 0.01)
      sub.monitor('ns=1;s=temp') { |dv| received << dv }

      deadline = Async::Clock.now + 2.0
      sleep 0.01 until received.size >= 1 ||
                           Async::Clock.now > deadline
      assert_operator received.size, :>=, 1
    end
  end
end
