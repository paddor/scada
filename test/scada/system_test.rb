require_relative "../test_helper"

describe "System tests" do
  include ScadaHelper

  # -- Helpers for restart tests (no shared server) --

  def make_server(port)
    cfg = Scada::Server::Config.default.with(port: port)
    cfg = cfg.with(logger: SILENT_LOGGER) if QUIET
    server = Scada::Server.new(config: cfg)
    server.add_variable('ns=1;s=temp', type: :double, value: 20.0,
                        display_name: 'Temperature')
    server.add_variable('ns=1;s=setpoint', type: :double, value: 10.0,
                        display_name: 'Setpoint')
    server.add_method('ns=1;s=greet',
      display_name: 'Greet',
      input: [{ name: 'name', type: :string }],
      output: [{ name: 'greeting', type: :string }]) { |req|
        "Hello, #{req[0]}!"
      }
    server
  end

  def wait_for_session(client, timeout: 5.0)
    deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + timeout
    loop do
      sleep 0.01
      s = client.state
      return true if s.session == Scada::Client::SESSION_ACTIVATED
      return false if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
    end
  end

  # -- Shared server for non-restart tests --

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=temp', type: :double, value: 20.0,
                    display_name: 'Temperature')
    s.add_variable('ns=1;s=setpoint', type: :double, value: 10.0,
                    display_name: 'Setpoint')
    s.add_variable('ns=1;s=flag', type: :boolean, value: false,
                    display_name: 'Flag')
    s.add_variable('ns=1;s=name', type: :string, value: 'initial',
                    display_name: 'Name')
    s.add_method('ns=1;s=greet',
      display_name: 'Greet',
      input: [{ name: 'name', type: :string }],
      output: [{ name: 'greeting', type: :string }]) { |req|
        "Hello, #{req[0]}!"
      }
    s.add_method('ns=1;s=double_it',
      display_name: 'DoubleIt',
      input: [{ name: 'x', type: :int32 }],
      output: [{ name: 'result', type: :int32 }]) { |req|
        req[0] * 2
      }
  }

  # ---- Multiple clients ----

  describe "multiple clients" do
    it "two clients read concurrently" do
      Sync do |task|
        c1 = new_client("opc.tcp://localhost:#{fixture.port}")
        c2 = new_client("opc.tcp://localhost:#{fixture.port}")
        c1.connect
        c2.connect
        t1 = task.async { c1.run }
        t2 = task.async { c2.run }

        r1 = c1.read('ns=1;s=temp').wait
        r2 = c2.read('ns=1;s=temp').wait
        assert r1.status.good?
        assert r2.status.good?
        assert_equal r1.value, r2.value
      ensure
        t1&.stop; t2&.stop
        c1&.close; c2&.close
      end
    end

    it "one client writes, another reads the updated value" do
      Sync do |task|
        writer = new_client("opc.tcp://localhost:#{fixture.port}")
        reader = new_client("opc.tcp://localhost:#{fixture.port}")
        writer.connect
        reader.connect
        wt = task.async { writer.run }
        rt = task.async { reader.run }

        writer.write('ns=1;s=setpoint', 77.0).wait
        dv = reader.read('ns=1;s=setpoint').wait
        assert_in_delta 77.0, dv.value, 0.01
      ensure
        wt&.stop; rt&.stop
        writer&.close; reader&.close
      end
    end

    it "one client writes, another gets notified via subscription" do
      Sync do |task|
        writer = new_client("opc.tcp://localhost:#{fixture.port}")
        observer = new_client("opc.tcp://localhost:#{fixture.port}")
        writer.connect
        observer.connect
        wt = task.async { writer.run }
        ot = task.async { observer.run }

        received = []
        sub = observer.subscribe(publish_interval: 0.01)
        sub.monitor('ns=1;s=flag') { |dv| received << dv.value }

        # wait for initial notification
        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received.size >= 1 || Async::Clock.now > deadline

        writer.write('ns=1;s=flag', true).wait

        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received.include?(true) || Async::Clock.now > deadline

        assert_includes received, true
      ensure
        wt&.stop; ot&.stop
        writer&.close; observer&.close
      end
    end
  end

  # ---- Concurrent async operations ----

  describe "concurrent async operations" do
    it "parallel reads within a single client" do
      with_client(fixture) do |client|
        tasks = %w[ns=1;s=temp ns=1;s=setpoint ns=1;s=flag ns=1;s=name].map do |nid|
          client.read(nid)
        end
        results = tasks.map(&:wait)
        assert_equal 4, results.length
        results.each { |dv| assert_instance_of Scada::DataValue, dv }
      end
    end

    it "interleaved reads and writes" do
      with_client(fixture) do |client|
        w = client.write('ns=1;s=name', 'concurrent')
        r = client.read('ns=1;s=temp')
        w.wait
        dv = r.wait
        assert_in_delta 20.0, dv.value, 0.01

        dv = client.read('ns=1;s=name').wait
        assert_equal 'concurrent', dv.value
      end
    end

    it "read and call in parallel" do
      with_client(fixture) do |client|
        read_task = client.read('ns=1;s=temp')
        call_task = client.call('ns=1;s=double_it', 21)

        dv = read_task.wait
        result = call_task.wait

        assert_in_delta 20.0, dv.value, 0.01
        assert_equal 42, result
      end
    end
  end

  # ---- Server restart ----

  describe "server restart" do
    it "client reads and calls after reconnect" do
      port = rand(40_000..50_000)

      Sync do |task|
        server1 = make_server(port)
        server_task = task.async { server1.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client("opc.tcp://localhost:#{port}", config: cfg)
        client.connect
        client_task = task.async { client.run }

        # verify initial connection
        dv = client.read('ns=1;s=temp').wait
        assert_in_delta 20.0, dv.value, 0.01

        # kill server
        server_task.stop
        server1.close
        sleep 0.2

        # start a new server on the same port
        server2 = make_server(port)
        server_task = task.async { server2.run }

        # wait for reconnection
        assert wait_for_session(client, timeout: 10.0),
               'Client should auto-reconnect'

        # read and call should work on the new server
        dv = client.read('ns=1;s=temp').wait
        assert_in_delta 20.0, dv.value, 0.01

        result = client.call('ns=1;s=greet', 'Reconnect').wait
        assert_equal 'Hello, Reconnect!', result
      ensure
        client_task&.stop
        client&.close
        server_task&.stop
        server2&.close
      end
    end

    it "subscriptions are lost after server restart" do
      port = rand(40_000..50_000)

      Sync do |task|
        server1 = make_server(port)
        server_task = task.async { server1.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client("opc.tcp://localhost:#{port}", config: cfg)
        client.connect
        client_task = task.async { client.run }

        # set up subscription and wait for initial notification
        received = []
        sub = client.subscribe(publish_interval: 0.01)
        sub.monitor('ns=1;s=temp') { |dv| received << dv.value }

        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received.size >= 1 || Async::Clock.now > deadline
        assert_operator received.size, :>=, 1, 'Should get initial notification'
        received.clear

        # kill server and restart
        server_task.stop
        server1.close
        sleep 0.2

        server2 = make_server(port)
        server_task = task.async { server2.run }

        assert wait_for_session(client, timeout: 10.0),
               'Client should auto-reconnect'

        # write a new value on the new server — old subscription should
        # NOT fire because subscription state is server-side
        client.write('ns=1;s=temp', 99.0).wait
        sleep 0.5

        assert_empty received,
                     'Old subscription should not fire after server restart'

        # but a new subscription works
        received2 = []
        sub2 = client.subscribe(publish_interval: 0.01)
        sub2.monitor('ns=1;s=temp') { |dv| received2 << dv.value }

        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received2.size >= 1 || Async::Clock.now > deadline
        assert_operator received2.size, :>=, 1,
                        'New subscription should work after reconnect'
      ensure
        client_task&.stop
        client&.close
        server_task&.stop
        server2&.close
      end
    end

    it "on_inactive fires after server restart" do
      port = rand(40_000..50_000)

      Sync do |task|
        server1 = make_server(port)
        server_task = task.async { server1.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client("opc.tcp://localhost:#{port}", config: cfg)
        client.connect
        client_task = task.async { client.run }

        # set up subscription with on_inactive callback
        inactive_called = false
        sub = client.subscribe(publish_interval: 0.01)
        sub.monitor('ns=1;s=temp') { |_dv| }
        sub.on_inactive { inactive_called = true }

        # wait for subscription to be active
        sleep 0.1

        # kill server and restart
        server_task.stop
        server1.close
        sleep 0.2

        server2 = make_server(port)
        server_task = task.async { server2.run }

        # wait for reconnection + inactivity detection
        deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 15.0
        until inactive_called
          sleep 0.05
          if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
            flunk 'on_inactive was not called within 15s'
          end
        end

        assert inactive_called, 'on_inactive should fire after server restart'
      ensure
        client_task&.stop
        client&.close
        server_task&.stop
        server2&.close
      end
    end

    it "on_inactive fires for each subscription independently" do
      port = rand(40_000..50_000)

      Sync do |task|
        server1 = make_server(port)
        server_task = task.async { server1.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client("opc.tcp://localhost:#{port}", config: cfg)
        client.connect
        client_task = task.async { client.run }

        inactive_ids = []

        sub1 = client.subscribe(publish_interval: 0.01)
        sub1.monitor('ns=1;s=temp') { |_dv| }
        sub1.on_inactive { inactive_ids << :sub1 }

        sub2 = client.subscribe(publish_interval: 0.01)
        sub2.monitor('ns=1;s=setpoint') { |_dv| }
        sub2.on_inactive { inactive_ids << :sub2 }

        sleep 0.1

        # kill server and restart
        server_task.stop
        server1.close
        sleep 0.2

        server2 = make_server(port)
        server_task = task.async { server2.run }

        deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 15.0
        until inactive_ids.size >= 2
          sleep 0.05
          if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
            flunk "Only #{inactive_ids.size}/2 on_inactive callbacks fired"
          end
        end

        assert_includes inactive_ids, :sub1
        assert_includes inactive_ids, :sub2
      ensure
        client_task&.stop
        client&.close
        server_task&.stop
        server2&.close
      end
    end

    it "on_inactive enables re-subscribing after server restart" do
      port = rand(40_000..50_000)

      Sync do |task|
        server1 = make_server(port)
        server_task = task.async { server1.run }

        cfg = Scada::Client::Config.default.with(
          connectivity_check_interval: 0.1
        )
        client = new_client("opc.tcp://localhost:#{port}", config: cfg)
        client.connect
        client_task = task.async { client.run }

        received = []
        needs_resubscribe = false

        sub = client.subscribe(publish_interval: 0.01)
        sub.monitor('ns=1;s=temp') { |dv| received << dv.value }
        # on_inactive fires during _run_iterate — cannot do async
        # operations here, so just set a flag for the main loop
        sub.on_inactive { needs_resubscribe = true }

        # wait for initial notification
        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received.size >= 1 || Async::Clock.now > deadline
        assert_operator received.size, :>=, 1, 'Should get initial notification'
        received.clear

        # kill server and restart
        server_task.stop
        server1.close
        sleep 0.2

        server2 = make_server(port)
        server_task = task.async { server2.run }

        # wait for on_inactive flag
        deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 15.0
        until needs_resubscribe
          sleep 0.05
          if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
            flunk 'on_inactive was not called within 15s'
          end
        end

        # wait for session to come back before re-subscribing
        assert wait_for_session(client, timeout: 10.0),
               'Client should auto-reconnect'

        # re-subscribe on the new session
        new_sub = client.subscribe(publish_interval: 0.01)
        new_sub.monitor('ns=1;s=temp') { |dv| received << dv.value }

        deadline = Async::Clock.now + 2.0
        sleep 0.01 until received.size >= 1 || Async::Clock.now > deadline
        assert_operator received.size, :>=, 1,
                        'Re-subscribed monitor should receive notifications'
      ensure
        client_task&.stop
        client&.close
        server_task&.stop
        server2&.close
      end
    end

    it "on_inactive does not fire during normal operation" do
      with_client(fixture) do |client|
        inactive_called = false
        sub = client.subscribe(publish_interval: 0.01)
        sub.monitor('ns=1;s=temp') { |_dv| }
        sub.on_inactive { inactive_called = true }

        # normal read/write activity
        client.write('ns=1;s=temp', 42.0).wait
        sleep 0.5

        refute inactive_called,
               'on_inactive should not fire during normal operation'
      end
    end
  end

  # ---- Mixed sync and async ----

  describe "mixed sync and async" do
    it "sync connect then async operations" do
      Sync do |task|
        client = new_client("opc.tcp://localhost:#{fixture.port}")
        # connect synchronously (no Fiber.scheduler at call time —
        # Sync sets one, but connect checks Fiber.scheduler which is
        # set inside Sync, so this will use async connect)
        client.connect
        client_task = task.async { client.run }

        dv = client.read('ns=1;s=temp').wait
        assert_instance_of Scada::DataValue, dv
        assert dv.status.good?

        client.write('ns=1;s=setpoint', 55.0).wait
        dv = client.read('ns=1;s=setpoint').wait
        assert_in_delta 55.0, dv.value, 0.01
      ensure
        client_task&.stop
        client&.close
      end
    end
  end

  # ---- Rapid connect/disconnect ----

  describe "rapid connect/disconnect" do
    it "connects and disconnects 10 times without leaking" do
      10.times do
        client = new_client("opc.tcp://localhost:#{fixture.port}")
        client.connect
        dv = client.read('ns=1;s=temp')
        assert_instance_of Scada::DataValue, dv
        client.close
      end
    end
  end

  # ---- Error propagation ----

  describe "error propagation" do
    it "connect to nonexistent server raises" do
      client = new_client("opc.tcp://localhost:1")
      assert_raises(Scada::Error) { client.connect }
    ensure
      client&.close
    end

    it "write to read-only node raises in async mode" do
      with_client(fixture) do |client|
        # Server object node is read-only
        assert_raises(Scada::Error) do
          client.write('i=2253', 42).wait  # ServerStatus node
        end
      end
    end

    it "call nonexistent method raises" do
      with_client(fixture) do |client|
        assert_raises(Scada::Error) do
          client.call('ns=1;s=nonexistent_method').wait
        end
      end
    end
  end
end
