require "async"
require "async/promise"

module Scada
  class Client
    Config = Data.define(
      :application_name, :application_uri, :product_uri,
      :certificate, :private_key, :trust_list,
      :security_mode, :username, :password,
      :logger
    ) do
      def self.default
        new(
          application_name: "Scada Ruby OPC UA Client",
          application_uri: "urn:scada:client",
          product_uri: "urn:scada.rb",
          certificate: nil, private_key: nil, trust_list: [],
          security_mode: SecurityMode::NONE,
          username: nil, password: nil,
          logger: nil
        )
      end

      def self.secure(certificate:, private_key:, **opts)
        default.with(
          certificate: certificate, private_key: private_key,
          security_mode: SecurityMode::SIGN_AND_ENCRYPT,
          **opts
        )
      end

      def self.development
        require "localhost"
        authority = Localhost::Authority.fetch
        cert = authority.client_identity.certificate
        key = authority.client_identity.key
        default.with(certificate: cert, private_key: key,
                     security_mode: SecurityMode::SIGN_AND_ENCRYPT)
      end
    end

    TICK = 0.05

    # Session states from open62541
    SESSION_ACTIVATED = 4

    def connect
      _connect_async
      # Iterate until session is activated
      deadline = Async::Clock.now + 5.0
      loop do
        _run_iterate
        session_state, connect_status, _channel = _get_state
        scada_check_connect_status(connect_status)
        break if session_state == SESSION_ACTIVATED
        raise Scada::Error, "Connect timed out" if Async::Clock.now > deadline
        sleep TICK
      end
      # v1.5: after session activation, the client reads the namespace
      # array asynchronously. A few more iterates complete the handshake.
      3.times { _run_iterate; sleep TICK }
      self
    end

    def run
      run_next = Async::Clock.now
      loop do
        run_next += TICK
        _run_iterate
        remaining = run_next - Async::Clock.now
        if remaining > 0
          sleep(remaining)
        else
          Async::Task.current.yield
        end
      end
    end

    def read(*node_ids, attribute: :value)
      nids = node_ids.map { |n| n.is_a?(NodeId) ? n : NodeId.parse(n.to_s) }
      if nids.length == 1
        Async do
          condition = Async::Promise.new
          _read_async(nids[0], attribute, condition)
          status, result = condition.wait
          check_async_status!(status)
          if attribute == :value
            result
          else
            result&.value
          end
        end
      else
        Async { nids.map { |nid| read(nid, attribute: attribute).wait } }
      end
    end

    def write(node_id, value)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id.to_s)
      Async do
        condition = Async::Promise.new
        _write_async(nid, value, condition)
        status, _result = condition.wait
        check_async_status!(status)
      end
    end

    def call(node_id, *args)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id.to_s)
      Async do
        condition = Async::Promise.new
        _call_async(nid, args, condition)
        status, result = condition.wait
        check_async_status!(status)
        result
      end
    end

    def namespace(uri)
      Namespace.new(self, uri)
    end

    def get_namespace_index(uri)
      _get_namespace_index(uri)
    end

    def subscribe(publish_interval: 0.1)
      condition = Async::Promise.new
      _create_subscription_async(publish_interval, condition)
      status, sub_id = condition.wait
      check_async_status!(status)
      Subscription.new(self, sub_id)
    end

    private

    def check_async_status!(status_code)
      return if status_code == 0
      raise Scada::Error, "OPC UA error: 0x#{status_code.to_s(16).rjust(8, '0')}"
    end

    def scada_check_connect_status(status_code)
      return if status_code == 0 # GOOD
      return if status_code == 0x00000001 # UNCERTAIN (in progress)
      # Raise for actual errors
      if (status_code & 0x80000000) != 0
        raise Scada::Error, "Connect failed: 0x#{status_code.to_s(16).rjust(8, '0')}"
      end
    end
  end
end

require_relative "client/namespace"
require_relative "client/subscription"
require_relative "client/monitored_item"
