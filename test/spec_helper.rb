require "minitest/autorun"
require "scada"
require "async"

QUIET = !ARGV.include?("--verbose")
SILENT_LOGGER = :silent

module ScadaHelper
  def with_server(config: nil)
    port = rand(40_000..50_000)
    cfg = config || Scada::Server::Config.default
    cfg = cfg.with(port: port)
    cfg = cfg.with(logger: SILENT_LOGGER) if QUIET && cfg.logger.nil?
    server = Scada::Server.new(config: cfg)
    yield server, port
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
