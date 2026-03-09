module Scada
  class Client
    class Subscription
      def initialize(client, sub_id)
        @client = client
        @sub_id = sub_id
      end

      def monitor_data_changes(node_id, trigger: nil, &block)
        nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id.to_s)
        condition = Async::Promise.new
        @client._add_monitored_data_change(@sub_id, nid, block, trigger, condition)
        status, mon_id = condition.wait
        @client.send(:check_async_status!, status)
        MonitoredItem.new(@client, @sub_id, mon_id)
      end
      alias_method :monitor, :monitor_data_changes

      def monitor_events(node_id, select: [], where: {}, &block)
        nid = node_id.is_a?(NodeId) ? node_id : NodeId.parse(node_id.to_s)
        condition = Async::Promise.new
        @client._add_monitored_event(@sub_id, nid, block, select, condition)
        status, mon_id = condition.wait
        @client.send(:check_async_status!, status)
        MonitoredItem.new(@client, @sub_id, mon_id)
      end
    end
  end
end
