require_relative '../spec_helper'

describe 'Connectivity check interval' do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable(
      'ns=1;s=val', type: :double, value: 1.0,
      display_name: 'Value'
    )
  }

  it 'client stays connected with connectivity check enabled' do
    cfg = Scada::Client::Config.default.with(
      connectivity_check_interval: 0.1
    )
    with_client(fixture, config: cfg) do |client|
      # Let several connectivity checks fire
      sleep 0.15

      # Client should still be connected and functional
      s = client.state
      assert_equal Scada::Client::SESSION_ACTIVATED, s.session

      dv = client.read('ns=1;s=val').wait
      assert_in_delta 1.0, dv.value, 0.01
    end
  end

  it 'client works without connectivity check (default)' do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=val').wait
      assert_in_delta 1.0, dv.value, 0.01
    end
  end
end
