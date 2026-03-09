require_relative "scada/version"

# Value objects must load before the native extension,
# which looks them up via rb_const_get during Init_scada.
require_relative "scada/data_value"
require_relative "scada/status"
require_relative "scada/event"
require_relative "scada/node_info"

# Native extension
require_relative "scada/scada"

# Pure-Ruby wrappers
require_relative "scada/node_id"
require_relative "scada/security_mode"
require_relative "scada/log_category"
require_relative "scada/server"
require_relative "scada/client"
