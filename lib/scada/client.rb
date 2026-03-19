module Scada
  class Client
    CONFIG_FIELDS = %i[
      application_name
      application_uri
      product_uri
      certificate
      private_key
      trust_list
      security_mode
      username
      password
      connectivity_check_interval
      logger
    ].freeze

    Config = Data.define(*CONFIG_FIELDS) do
      def self.default
        new(
          application_name:           'Scada Ruby OPC UA Client',
          application_uri:            'urn:scada:client',
          product_uri:                'urn:scada.rb',
          certificate:                nil,
          private_key:                nil,
          trust_list:                 [],
          security_mode:              SecurityMode::NONE,
          username:                   nil,
          password:                   nil,
          connectivity_check_interval: nil,
          logger:                     nil
        )
      end

      def self.secure(certificate:, private_key:, **opts)
        default.with(
          certificate:   certificate,
          private_key:   private_key,
          security_mode: SecurityMode::SIGN_AND_ENCRYPT,
          **opts
        )
      end

      def self.development
        require 'localhost'
        authority = Localhost::Authority.fetch
        cert      = authority.client_identity.certificate
        key       = authority.client_identity.key
        default.with(
          certificate:   cert,
          private_key:   key,
          security_mode: SecurityMode::SIGN_AND_ENCRYPT
        )
      end
    end

    # Minimum yield interval for the event loop.
    # Allows other fibers to run without busy-looping.
    TICK = 0.001

    # Session states from open62541
    SESSION_ACTIVATED = 4

    # Snapshot of the client's connection state.
    #
    # @!attribute [r] session [Integer] session state (see +SESSION_ACTIVATED+)
    # @!attribute [r] status [StatusCode] connection status code
    # @!attribute [r] channel [Integer] secure channel state
    #
    State = Data.define(:session, :status, :channel) do
      # @return [Boolean] whether the OPC UA session is fully activated
      def session_activated?
        session == SESSION_ACTIVATED
      end
    end

    # @return [State] the current connection state
    def state
      session, status_code, channel = _get_state
      State.new(session, StatusCode.new(status_code), channel)
    end

    # Registers a callback that fires when the session becomes activated inside the {#run} loop.
    #
    # Fires on the initial connection and on every subsequent reconnection. The callback is
    # invoked from within +_run_iterate+, so it must not perform blocking SCADA operations
    # (use a deferred queue or flag instead).
    #
    # @yield called when session transitions from inactive → activated
    #
    def on_connect(&block)
      @on_connect = block
    end

    # Registers a callback that fires when the session drops inside the {#run} loop.
    #
    # Fires when a previously activated session becomes inactive (e.g. server stopped).
    # Same restrictions as {#on_connect}: no blocking SCADA operations in the callback.
    #
    # @yield called when session transitions from activated → inactive
    #
    def on_disconnect(&block)
      @on_disconnect = block
    end

    def connect
      _connect_sync
      unless _have_namespaces?
        deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 5.0
        wait_for_namespaces(deadline)
      end
      @was_connected = true
      self
    end

    def run
      raise Scada::Error, '#run requires an Async context'
    end

    def read(*node_ids, attribute: :value)
      nids = node_ids.map { |n| parse_node_id(n) }
      results = nids.map { |nid| read_single_sync(nid, attribute) }
      results.length == 1 ? results[0] : results
    end

    def write(node_id, value, type: nil)
      nid = parse_node_id(node_id)
      status, _result = _write_sync(nid, value, type)
      check_async_status!(status)
      nil
    end

    def call(node_id, *args)
      nid = parse_node_id(node_id)
      status, result = _call_sync(nid, args)
      check_async_status!(status)
      result
    end

    def namespace(uri)
      Namespace.new(self, uri)
    end

    def get_namespace_index(uri)
      _get_namespace_index(uri)
    end

    def subscribe(publish_interval: 0.1) # rubocop:disable Lint/UnusedMethodArgument
      raise Scada::Error, '#subscribe requires an Async context'
    end

    private

    def parse_node_id(node_id)
      if node_id.is_a?(NodeId)
        node_id
      else
        NodeId.parse(node_id.to_s)
      end
    end

    def read_single_sync(nid, attribute)
      status, result = _read_sync(nid, attribute)
      check_async_status!(status)
      attribute == :value ? result : result&.value
    end

    def check_async_status!(status_code)
      return if status_code == 0

      Scada::Error.raise_for_code(status_code)
    end

    def raise_on_bad_status!(status, prefix)
      return unless status.bad?

      raise Scada::Error, "#{prefix}: #{status}"
    end

    def wait_for_namespaces(deadline)
      loop do
        _run_iterate
        break if _have_namespaces?
        if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
          raise Scada::Error, 'Namespace fetch timed out'
        end
        sleep TICK
      end
    end
  end
end

require_relative 'client/namespace'
