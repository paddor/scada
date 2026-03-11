module Scada
  class Error
    @by_code = {}

    class << self
      attr_reader :by_code

      def raise_for_code(code)
        entry = @by_code[code]
        if entry
          raise entry[0], entry[1]
        else
          raise self, format("OPC UA error: 0x%08x", code)
        end
      end
    end

    csv_path = File.expand_path("../../data/StatusCode.csv", __dir__)
    File.foreach(csv_path) do |line|
      line.chomp!
      name, value, description = line.split(",", 3)
      next if name.nil? || value.nil?
      next if %w[Good Uncertain Bad].include?(name)

      description = description&.delete_prefix('"')&.delete_suffix('"') || ""
      code = Integer(value)
      klass = Class.new(self)
      const_set(name, klass)
      klass.instance_variable_set(:@status_code, code)
      @by_code[code] = [klass, description].freeze
    end

    @by_code.freeze
  end
end
