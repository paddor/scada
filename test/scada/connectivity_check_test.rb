require_relative '../spec_helper'

describe 'Connectivity check interval' do
  include ScadaHelper

  it 'client stays connected with connectivity check enabled' do
    with_server do |server, port|
      server.add_variable(
        'ns=1;s=val', type: :double, value: 1.0,
        display_name: 'Value'
      )

      Sync do |task|
        server_task = task.async { server.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client(
          "opc.tcp://localhost:#{port}", config: cfg
        )
        client.connect
        client_task = task.async { client.run }

        # Let several connectivity checks fire
        sleep 0.5

        # Client should still be connected and functional
        s = client.state
        assert_equal Scada::Client::SESSION_ACTIVATED,
                     s.session

        dv = client.read('ns=1;s=val').wait
        assert_in_delta 1.0, dv.value, 0.01
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it 'client works without connectivity check (default)' do
    with_server do |server, port|
      server.add_variable(
        'ns=1;s=val', type: :double, value: 2.0,
        display_name: 'Value'
      )

      Sync do |task|
        server_task = task.async { server.run }

        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv = client.read('ns=1;s=val').wait
        assert_in_delta 2.0, dv.value, 0.01
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
