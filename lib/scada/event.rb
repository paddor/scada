module Scada
  Event = Data.define(:event_type, :message, :severity, :source_name, :time)
end
