require_relative '../spec_helper'

describe 'Auto-reconnect' do
  include ScadaHelper

  def make_server(port)
    cfg = Scada::Server::Config.default.with(port: port)
    cfg = cfg.with(logger: SILENT_LOGGER) if QUIET
    server = Scada::Server.new(config: cfg)
    server.add_variable(
      'ns=1;s=val', type: :double, value: 1.0,
      display_name: 'Value'
    )
    server
  end

  def wait_for_session(client, timeout: 5.0)
    deadline = Async::Clock.now + timeout
    loop do
      sleep 0.1
      s = client.state
      return true if s.session ==
                      Scada::Client::SESSION_ACTIVATED
      return false if Async::Clock.now > deadline
    end
  end

  it 'state.status returns a Scada::StatusCode' do
    with_server do |server, port|
      server.add_variable(
        'ns=1;s=val', type: :double, value: 1.0,
        display_name: 'Value'
      )

      Sync do |task|
        server_task = task.async { server.run }

        client = new_client(
          "opc.tcp://localhost:#{port}"
        )
        client.connect
        client_task = task.async { client.run }

        s = client.state
        assert_instance_of Scada::StatusCode, s.status
        assert s.status.good?,
               "Status should be good: #{s.status}"
        refute s.status.bad?
        refute s.status.uncertain?
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it 'reconnects after server restart' do
    port = rand(40_000..50_000)

    Sync do |task|
      # Start first server
      server1 = make_server(port)
      server_task = task.async { server1.run }

      # Connect client with fast connectivity check
      cfg = Scada::Client::Config.default.with(
        connectivity_check_interval: 0.1
      )
      client = new_client(
        "opc.tcp://localhost:#{port}", config: cfg
      )
      client.connect
      client_task = task.async { client.run }

      # Verify initial connection works
      dv = client.read('ns=1;s=val').wait
      assert_in_delta 1.0, dv.value, 0.01

      # Kill the server
      server_task.stop
      sleep 0.5

      # Client should notice the disconnect
      s = client.state
      refute_equal Scada::Client::SESSION_ACTIVATED,
                   s.session,
                   'Session should not be active ' \
                   'after server stop'

      # Start a new server on the same port
      server2 = make_server(port)
      server_task = task.async { server2.run }

      # Wait for auto-reconnect
      reconnected = wait_for_session(
        client, timeout: 10.0
      )
      assert reconnected,
             'Client should auto-reconnect'

      # Verify status is good after reconnect
      s = client.state
      assert_instance_of Scada::StatusCode, s.status
      assert s.status.good?,
             "Status should be good: #{s.status}"

      # Verify the reconnected client can read
      dv = client.read('ns=1;s=val').wait
      assert_in_delta 1.0, dv.value, 0.01
    ensure
      client_task&.stop
      server_task&.stop
    end
  end
end
