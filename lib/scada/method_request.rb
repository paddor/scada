# frozen_string_literal: true

module Scada
  METHOD_REQUEST_FIELDS = %i[
    input_arguments session_id method_id object_node_id
  ].freeze

  MethodRequest = Data.define(*METHOD_REQUEST_FIELDS) do
    def [](index)
      input_arguments[index]
    end
  end
end
