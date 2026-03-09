require_relative "../spec_helper"

# Tests for the StatusValueTimestamp data change trigger and
# Server#write_value with explicit sourceTimestamp.
#
# Background: open62541 issue #5882 — in older versions, variables without
# an explicit sourceTimestamp got "now" filled in on every read, causing
# a flood of subscription notifications with StatusValueTimestamp trigger.
# The workaround is Server#write_value which sets a DataValue with an
# explicit sourceTimestamp. v1.5 appears to have improved this, but
# write_value remains useful for controlling timestamps precisely.

describe "Timestamp trigger (issue #5882)" do
  include ScadaHelper

  it "write_value pins the sourceTimestamp" do
    with_server do |server, port|
      server.add_variable("ns=1;s=pinned", type: :double, value: 42.0,
                          display_name: "Pinned")
      server.write_value("ns=1;s=pinned", 42.0, type: :double)

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv1 = client.read("ns=1;s=pinned").wait
        sleep 0.1
        dv2 = client.read("ns=1;s=pinned").wait

        assert_in_delta 42.0, dv1.value, 0.01
        assert_equal dv1.source_timestamp, dv2.source_timestamp,
          "sourceTimestamp should be stable across reads"
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "write_value updates the sourceTimestamp when the value changes" do
    with_server do |server, port|
      server.add_variable("ns=1;s=temp", type: :double, value: 20.0,
                          display_name: "Temperature")
      server.write_value("ns=1;s=temp", 20.0, type: :double)

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv1 = client.read("ns=1;s=temp").wait
        assert_in_delta 20.0, dv1.value, 0.01

        sleep 0.05
        # Server-side update with new timestamp
        server.write_value("ns=1;s=temp", 25.0, type: :double)

        dv2 = client.read("ns=1;s=temp").wait
        assert_in_delta 25.0, dv2.value, 0.01
        refute_equal dv1.source_timestamp, dv2.source_timestamp,
          "sourceTimestamp should change after write_value"
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "status_value_timestamp trigger fires on value change" do
    with_server do |server, port|
      server.add_variable("ns=1;s=val", type: :double, value: 10.0,
                          display_name: "Value")
      server.write_value("ns=1;s=val", 10.0, type: :double)

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        received = []
        sub = client.subscribe(publish_interval: 0.05)
        sub.monitor_data_changes("ns=1;s=val",
          trigger: :status_value_timestamp) { |dv| received << dv }

        # Wait for initial notification
        sleep 0.3
        initial_count = received.size
        assert_operator initial_count, :>=, 1, "Should get initial notification"

        # Write a new value — should trigger a notification
        client.write("ns=1;s=val", 99.0).wait
        sleep 0.3

        assert_operator received.size, :>, initial_count,
          "Should get notification after value change"
        assert_in_delta 99.0, received.last.value, 0.01
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "status_value_timestamp trigger does not flood with pinned timestamp" do
    with_server do |server, port|
      server.add_variable("ns=1;s=stable", type: :double, value: 42.0,
                          display_name: "Stable")
      server.write_value("ns=1;s=stable", 42.0, type: :double)

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        received = []
        sub = client.subscribe(publish_interval: 0.05)
        sub.monitor_data_changes("ns=1;s=stable",
          trigger: :status_value_timestamp) { |dv| received << dv }

        # Wait for initial notification
        sleep 0.3
        initial_count = received.size

        # Wait longer — no writes, timestamp is pinned, no flood
        sleep 1.0

        assert_equal initial_count, received.size,
          "Should not receive extra notifications when nothing changes"
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
