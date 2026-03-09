require "async"

module Scada
  class Server
    TICK = 0.02

    Config = Data.define(
      :port, :application_name, :application_uri, :product_uri,
      :certificate, :private_key, :trust_list,
      :security_mode, :users, :allow_anonymous,
      :sampling_interval, :publishing_interval,
      :logger
    ) do
      def self.default
        new(
          port: 4840,
          application_name: "Scada Ruby OPC UA Server",
          application_uri: "urn:scada:server",
          product_uri: "urn:scada.rb",
          certificate: nil, private_key: nil, trust_list: [],
          security_mode: SecurityMode::NONE,
          users: {}, allow_anonymous: nil,
          sampling_interval: 0.005..10.0,
          publishing_interval: 0.01..60.0,
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
        cert = authority.server_identity.certificate
        key = authority.server_identity.key
        default.with(
          certificate: cert, private_key: key,
          security_mode: SecurityMode::SIGN_AND_ENCRYPT,
          application_uri: cert.extensions.find { |e|
            e.oid == "subjectAltName"
          }&.value&.[](/URI:(.+)/, 1) || "urn:scada:server"
        )
      end
    end

    def run
      _run_startup
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
    ensure
      _run_shutdown
    end

    def add_variable(node_id_str, type:, value: nil, display_name: nil, browse_name: nil,
                     on_read: nil, on_write: nil, &write_validator)
      nid = node_id_str.is_a?(NodeId) ? node_id_str : NodeId.parse(node_id_str)
      dn = display_name || browse_name || node_id_str.to_s
      bn = browse_name || (display_name.is_a?(String) ? display_name : node_id_str.to_s)

      if on_read || on_write
        _add_data_source_variable(nid, type, dn, bn, on_read, on_write)
      else
        _add_variable_node(nid, type, value, dn, bn, write_validator)
      end
    end

    def add_object(node_id_str, display_name: nil, browse_name: nil,
                   parent: Scada::NS0::OBJECTS_FOLDER)
      nid = node_id_str.is_a?(NodeId) ? node_id_str : NodeId.parse(node_id_str)
      dn = display_name || node_id_str.to_s
      bn = browse_name || dn
      _add_object_node(nid, dn, bn, parent)
    end

    def add_method(node_id_str, display_name: nil, browse_name: nil,
                   input: [], output: [], &block)
      nid = node_id_str.is_a?(NodeId) ? node_id_str : NodeId.parse(node_id_str)
      dn = display_name || node_id_str.to_s
      bn = browse_name || dn
      _add_method_node(nid, dn, bn, input, output, block)
    end

    def load_nodeset(path)
      _load_nodeset(path.to_s)
    end

    def namespace(uri)
      Client::Namespace.new(self, uri)
    end

    def on_read(node_id, &block)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id)
      _on_read(nid, block)
    end

    def on_write(node_id, &block)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id)
      _on_write(nid, block)
    end

    def on_call(node_id, &block)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id)
      _on_call(nid, block)
    end

    def bind(ns, &block)
      BindContext.new(self, ns).instance_eval(&block)
    end

    def write_value(node_id, value, type:)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id.to_s)
      _write_value(nid, value, type)
    end

    def add_namespace(uri)
      _add_namespace(uri)
    end

    def has_node?(node_id)
      nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id)
      _has_node(nid)
    end

    def each_node(namespace: nil, node_class: nil, &block)
      _each_node(namespace, node_class, &block)
    end

    class BindContext
      def initialize(server, ns)
        @server = server
        @ns = ns
      end

      def on_read(name, &block)
        @server.on_read(@ns[name], &block)
      end

      def on_write(name, &block)
        @server.on_write(@ns[name], &block)
      end

      def on_call(name, &block)
        @server.on_call(@ns[name], &block)
      end
    end
  end
end
