require_relative "../test_helper"

describe "Data Sources" do
  include ScadaHelper

  READ_EVENTS = Queue.new
  WRITTEN_VALUES = Queue.new

  before do
    READ_EVENTS.pop until READ_EVENTS.empty?
    WRITTEN_VALUES.pop until WRITTEN_VALUES.empty?
  end

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=sensor', type: :double,
                    display_name: 'Sensor',
                    on_read: -> { READ_EVENTS.push(true); 42.5 })

    s.add_variable('ns=1;s=actuator', type: :double,
                    display_name: 'Actuator',
                    on_read: -> { 0.0 },
                    on_write: ->(_old_dv, new_dv) {
                      WRITTEN_VALUES.push(new_dv.value)
                    })
  }

  it "calls on_read callback when client reads" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=sensor').wait
      assert_in_delta 42.5, dv.value, 0.01
      assert_operator READ_EVENTS.size, :>=, 1
    end
  end

  it "calls on_write callback when client writes" do
    with_client(fixture) do |client|
      client.write('ns=1;s=actuator', 99.0).wait
      sleep 0.05
      assert_in_delta 99.0, WRITTEN_VALUES.pop, 0.01
    end
  end
end
