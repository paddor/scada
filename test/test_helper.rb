require "minitest/autorun"
require "scada"
require "async"

# Suppress "IO::Buffer is experimental" warning from console gem
Warning[:experimental] = false

QUIET = !ARGV.include?("--verbose")
SILENT_LOGGER = :silent

module ScadaHelper
  SharedServer = Data.define(:server, :port)

  # Start a shared server in a background Thread.
  # The server persists for the entire test run. Use this at the
  # describe-block level to share one server across many tests.
  @shared_servers = []

  def self.start_shared_server(config: nil, &setup)
    port = rand(30_000..60_000)
    cfg = config || Scada::Server::Config.default
    cfg = cfg.with(port: port)
    cfg = cfg.with(logger: SILENT_LOGGER) if QUIET && cfg.logger.nil?
    server = Scada::Server.new(config: cfg)
    setup.call(server) if setup
    thread = Thread.new { Sync { server.run } }
    thread.abort_on_exception = true
    sleep 0.02 # let the server bind
    @shared_servers << [server, thread]
    SharedServer.new(server: server, port: port)
  end

  Minitest.after_run do
    @shared_servers.each { |_, t| t.kill }
    @shared_servers.each { |_, t| t.join(1) }
    @shared_servers.each { |s, _| s.close }
  end

  # Create a short-lived server for one test (old pattern).
  def with_server(config: nil)
    port = rand(30_000..60_000)
    cfg = config || Scada::Server::Config.default
    cfg = cfg.with(port: port)
    cfg = cfg.with(logger: SILENT_LOGGER) if QUIET && cfg.logger.nil?
    server = Scada::Server.new(config: cfg)
    yield server, port
  ensure
    server.close
  end

  # Connect a client to a SharedServer, run its event loop,
  # and yield it. Cleans up automatically.
  def with_client(fixture, config: nil)
    Sync do |task|
      client = new_client(
        "opc.tcp://localhost:#{fixture.port}", config: config
      )
      client.connect
      client_task = task.async { client.run }
      yield client
    ensure
      client_task&.stop
      client&.close
    end
  end

  # Access the shared server fixture stored on the describe class.
  # Walks up the class hierarchy so nested describe blocks inherit it.
  def fixture
    klass = self.class
    while klass
      f = klass.instance_variable_get(:@fixture)
      return f if f
      klass = klass.superclass
    end
    nil
  end

  def new_client(url, config: nil)
    if config
      config = config.with(logger: SILENT_LOGGER) if QUIET && config.logger.nil?
      Scada::Client.new(url, config: config)
    elsif QUIET
      Scada::Client.new(url, config: Scada::Client::Config.default.with(logger: SILENT_LOGGER))
    else
      Scada::Client.new(url)
    end
  end
end
