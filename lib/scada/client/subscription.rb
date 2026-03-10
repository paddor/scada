module Scada
  class Client
    class Subscription
      def initialize(client, sub_id)
        @client = client
        @sub_id = sub_id
      end

      def monitor_data_changes(node_id, trigger: nil, &block)
        nid = parse_node_id(node_id)
        condition = Async::Promise.new
        @client._add_monitored_data_change(
          @sub_id, nid, block, trigger, condition
        )
        status, mon_id = condition.wait
        @client.send(:check_async_status!, status)
        MonitoredItem.new(@client, @sub_id, mon_id)
      end
      alias_method :monitor, :monitor_data_changes

      def monitor_events(node_id, select: [], where: {}, &block)
        nid = parse_node_id(node_id)
        condition = Async::Promise.new
        @client._add_monitored_event(
          @sub_id, nid, block, select, condition
        )
        status, mon_id = condition.wait
        @client.send(:check_async_status!, status)
        MonitoredItem.new(@client, @sub_id, mon_id)
      end

      private

      def parse_node_id(node_id)
        if node_id.is_a?(NodeId)
          node_id
        else
          NodeId.parse(node_id.to_s)
        end
      end
    end
  end
end
