module Scada
  DATAVALUE_FIELDS = %i[
    value
    status
    source_timestamp
    server_timestamp
  ].freeze

  DataValue = Data.define(*DATAVALUE_FIELDS)
end
