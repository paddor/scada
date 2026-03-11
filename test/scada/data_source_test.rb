require_relative "../spec_helper"

describe "Data Sources" do
  include ScadaHelper

  # Data sources use callbacks, so state is per-server — safe to share
  READ_COUNT = [0]
  WRITTEN_VALUE = [nil]

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=sensor', type: :double,
                    display_name: 'Sensor',
                    on_read: -> { READ_COUNT[0] += 1; 42.5 })

    s.add_variable('ns=1;s=actuator', type: :double,
                    display_name: 'Actuator',
                    on_read: -> { 0.0 },
                    on_write: ->(_old_dv, new_dv) {
                      WRITTEN_VALUE[0] = new_dv.value
                    })
  }

  it "calls on_read callback when client reads" do
    with_client(fixture) do |client|
      READ_COUNT[0] = 0
      dv = client.read('ns=1;s=sensor').wait
      assert_in_delta 42.5, dv.value, 0.01
      assert_operator READ_COUNT[0], :>=, 1
    end
  end

  it "calls on_write callback when client writes" do
    with_client(fixture) do |client|
      WRITTEN_VALUE[0] = nil
      client.write('ns=1;s=actuator', 99.0).wait
      sleep 0.05
      assert_in_delta 99.0, WRITTEN_VALUE[0], 0.01
    end
  end
end
