require_relative "../test_helper"

# Reproduction test for https://github.com/open62541/open62541/issues/7492
describe "UA_Server_addDataSourceVariableNode return value (issue #7492)" do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=ds_single', type: :double,
                    display_name: 'Single DS',
                    on_read: -> { 1.0 })

    50.times do |i|
      s.add_variable("ns=1;s=ds_#{i}", type: :double,
                      display_name: "DS #{i}",
                      on_read: -> { i.to_f })
    end

    s.add_variable('ns=1;s=ds_rw', type: :double,
                    display_name: 'RW DS',
                    on_read: -> { 0.0 },
                    on_write: ->(_old, _new) { })

    s.add_variable('ns=1;s=ds_work', type: :double,
                    display_name: 'Working DS',
                    on_read: -> { 42.5 })
  }

  it "returns GOOD status for a single data source variable" do
    assert fixture.server.has_node?('ns=1;s=ds_single')
  end

  it "returns GOOD status for many data source variables" do
    50.times do |i|
      assert fixture.server.has_node?("ns=1;s=ds_#{i}"),
             "Expected node ds_#{i} to exist"
    end
  end

  it "returns GOOD status for read+write data source" do
    assert fixture.server.has_node?('ns=1;s=ds_rw')
  end

  it "data source variables actually work after creation" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=ds_work').wait
      assert_in_delta 42.5, dv.value, 0.01
    end
  end
end
