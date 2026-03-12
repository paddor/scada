require 'async'

module Scada
  class Server
    CONFIG_FIELDS = %i[
      port application_name application_uri product_uri
      certificate private_key trust_list
      security_mode users allow_anonymous
      sampling_interval publishing_interval
      logger
    ].freeze

    Config = Data.define(*CONFIG_FIELDS) do
      def self.default
        new(
          port:                4840,
          application_name:    'Scada Ruby OPC UA Server',
          application_uri:     'urn:scada:server',
          product_uri:         'urn:scada.rb',
          certificate:         nil,
          private_key:         nil,
          trust_list:          [],
          security_mode:       SecurityMode::NONE,
          users:               {},
          allow_anonymous:     nil,
          sampling_interval:   0.005..10.0,
          publishing_interval: 0.01..60.0,
          logger:              nil
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
        cert      = authority.server_identity.certificate
        key       = authority.server_identity.key
        san       = cert.extensions.find do |e|
          e.oid == 'subjectAltName'
        end
        uri = san&.value&.[](/URI:(.+)/, 1)
        default.with(
          certificate:     cert,
          private_key:     key,
          security_mode:   SecurityMode::SIGN_AND_ENCRYPT,
          application_uri: uri || 'urn:scada:server'
        )
      end
    end

    # Minimum yield interval for the event loop.
    TICK = 0.001

    def run
      _run_startup
      loop do
        _run_iterate
        _drain_async_completions
        sleep TICK
      end
    ensure
      _run_shutdown
    end

    def add_variable( # rubocop:disable Metrics/ParameterLists
      node_id_str, type:,
      value: nil, display_name: nil, browse_name: nil,
      on_read: nil, on_write: nil, &write_validator
    )
      nid = parse_node_id(node_id_str)
      dn = display_name || browse_name || node_id_str.to_s
      bn = browse_name || default_browse_name(
        display_name, node_id_str
      )

      if on_read || on_write
        _add_data_source_variable(
          nid, type, dn, bn, on_read, on_write
        )
      else
        _add_variable_node(
          nid, type, value, dn, bn, write_validator
        )
      end
    end

    def add_object(
      node_id_str, display_name: nil, browse_name: nil,
      parent: Scada::NS0::OBJECTS_FOLDER
    )
      nid = parse_node_id(node_id_str)
      dn = display_name || node_id_str.to_s
      bn = browse_name || dn
      _add_object_node(nid, dn, bn, parent)
    end

    def add_method(
      node_id_str, display_name: nil, browse_name: nil,
      input: [], output: [], &block
    )
      nid = parse_node_id(node_id_str)
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
      _on_read(parse_node_id(node_id), block)
    end

    def on_write(node_id, &block)
      _on_write(parse_node_id(node_id), block)
    end

    def on_call(node_id, &block)
      _on_call(parse_node_id(node_id), block)
    end

    def bind(ns, &block)
      BindContext.new(self, ns).instance_eval(&block)
    end

    def update_value(node_id, value, type:, timestamp: Time.now)
      _write_value(parse_node_id(node_id), value, type, timestamp.to_f)
    end

    def add_namespace(uri)
      _add_namespace(uri)
    end

    def has_node?(node_id) # rubocop:disable Naming/PredicateName
      _has_node(parse_node_id(node_id))
    end

    def each_node(namespace: nil, node_class: nil, &block)
      _each_node(namespace, node_class, &block)
    end

    private

    def _schedule_async_completion(task, output_ptr, output_type)
      (@_pending_async_completions ||= []) << [task, output_ptr, output_type]
    end

    def _drain_async_completions
      return unless defined?(@_pending_async_completions) && @_pending_async_completions&.any?
      @_pending_async_completions.each do |task, output_ptr, output_type|
        Async do
          result = task.wait
          _commit_async_method_result(output_ptr, result, output_type)
        rescue
          _fail_async_method_result(output_ptr)
        end
      end
      @_pending_async_completions.clear
    end

    def parse_node_id(node_id)
      if node_id.is_a?(NodeId)
        node_id
      else
        NodeId.parse(node_id.to_s)
      end
    end

    def default_browse_name(display_name, node_id_str)
      if display_name.is_a?(String)
        display_name
      else
        node_id_str.to_s
      end
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
