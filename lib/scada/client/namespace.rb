module Scada
  class Client
    class Namespace
      def initialize(owner, uri)
        @owner = owner
        @uri = uri
        @index = nil
      end

      def index
        @index ||= @owner._namespace_get_index(@uri)
      end

      def [](name)
        NodeId.parse("ns=#{index};s=#{name}")
      end
    end
  end
end
