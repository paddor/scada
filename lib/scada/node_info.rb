module Scada
  NODE_INFO_FIELDS = %i[
    node_id
    node_class
    display_name
    browse_name
    parent
    organizes
  ].freeze

  NodeInfo = Data.define(*NODE_INFO_FIELDS)
end
