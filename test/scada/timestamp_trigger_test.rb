require_relative "../spec_helper"

describe "Timestamp trigger (issue #5882)" do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=pinned', type: :double, value: 42.0,
                    display_name: 'Pinned')
    s.write_value('ns=1;s=pinned', 42.0, type: :double)

    s.add_variable('ns=1;s=temp', type: :double, value: 20.0,
                    display_name: 'Temperature')
    s.write_value('ns=1;s=temp', 20.0, type: :double)

    s.add_variable('ns=1;s=val', type: :double, value: 10.0,
                    display_name: 'Value')
    s.write_value('ns=1;s=val', 10.0, type: :double)
  }

  it "write_value pins the sourceTimestamp" do
    with_client(fixture) do |client|
      dv1 = client.read('ns=1;s=pinned').wait
      sleep 0.01
      dv2 = client.read('ns=1;s=pinned').wait

      assert_in_delta 42.0, dv1.value, 0.01
      assert_equal dv1.source_timestamp, dv2.source_timestamp,
        'sourceTimestamp should be stable across reads'
    end
  end

  it "write_value updates the sourceTimestamp when the value changes" do
    with_client(fixture) do |client|
      dv1 = client.read('ns=1;s=temp').wait
      assert_in_delta 20.0, dv1.value, 0.01

      sleep 0.01
      fixture.server.write_value('ns=1;s=temp', 25.0, type: :double)

      dv2 = client.read('ns=1;s=temp').wait
      assert_in_delta 25.0, dv2.value, 0.01
      refute_equal dv1.source_timestamp, dv2.source_timestamp,
        'sourceTimestamp should change after write_value'
    end
  end

  it "status_value_timestamp trigger fires on value change" do
    with_client(fixture) do |client|
      received = []
      sub = client.subscribe(publish_interval: 0.01)
      sub.monitor_data_changes('ns=1;s=val',
        trigger: :status_value_timestamp) { |dv| received << dv }

      # Wait for initial notification
      deadline = Async::Clock.now + 2.0
      sleep 0.01 until received.size >= 1 ||
                           Async::Clock.now > deadline
      initial_count = received.size
      assert_operator initial_count, :>=, 1,
        'Should get initial notification'

      # Write a new value — should trigger a notification
      client.write('ns=1;s=val', 99.0).wait
      deadline = Async::Clock.now + 2.0
      sleep 0.01 until received.size > initial_count ||
                           Async::Clock.now > deadline

      assert_operator received.size, :>, initial_count,
        'Should get notification after value change'
      assert_in_delta 99.0, received.last.value, 0.01
    end
  end
end
