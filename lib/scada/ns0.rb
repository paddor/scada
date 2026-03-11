module Scada
  csv_path = File.expand_path("../../data/NodeIds.csv", __dir__)
  File.foreach(csv_path) do |line|
    line.chomp!
    name, id, category = line.split(",", 3)
    next if name.nil? || id.nil?

    const_name = name
      .gsub(/([a-z\d])([A-Z])/, '\1_\2')
      .gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2')
      .upcase

    node_id = NodeId.parse("i=#{id}")
    NS0.const_set(const_name, node_id)
    DataType.const_set(const_name, node_id) if category&.strip == "DataType"
  end
end
