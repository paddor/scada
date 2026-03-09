module Scada
  class Client
    class MonitoredItem
      attr_reader :id

      def initialize(client, sub_id, mon_id)
        @client = client
        @sub_id = sub_id
        @id = mon_id
      end

      def delete
        @client._delete_monitored_item(@sub_id, @id)
      end
    end
  end
end
