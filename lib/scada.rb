require_relative 'scada/version'

# Value objects must load before the native extension,
# which looks them up via rb_const_get during Init_scada.
require_relative 'scada/data_value'
require_relative 'scada/status'
require_relative 'scada/event'
require_relative 'scada/node_info'
require_relative 'scada/method_request'

# Native extension
require_relative 'scada/scada'

# Error subclasses from CSV (must load after native ext defines Scada::Error)
require_relative 'scada/errors'

# NS0 and DataType constants from CSV (needs NodeId from native ext)
require_relative 'scada/ns0'

# Pure-Ruby wrappers
require_relative 'scada/node_id'
require_relative 'scada/security_mode'
require_relative 'scada/log_category'
require_relative 'scada/server'
require_relative 'scada/client'
