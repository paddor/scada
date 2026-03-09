#!/usr/bin/env ruby
# frozen_string_literal: true

# Generates ns0_ids.h and status_codes.h from open62541 CSV files.
# Usage: ruby script/generate_headers.rb [open62541_src_dir]

require "csv"
require "fileutils"

SRC_DIR = ARGV[0] || File.expand_path("../../open62541-src", __dir__)
OUT_DIR = File.expand_path("../ext/scada/generated", __dir__)
FileUtils.mkdir_p(OUT_DIR)

# --- CamelCase to UPPER_SNAKE_CASE ---
def to_upper_snake(name)
  name
    .gsub(/([a-z\d])([A-Z])/, '\1_\2')
    .gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2')
    .upcase
end

# --- ns0_ids.h ---
node_ids_csv = File.join(SRC_DIR, "tools/schema/NodeIds.csv")
abort "NodeIds.csv not found at #{node_ids_csv}" unless File.exist?(node_ids_csv)

ns0_lines = []
dt_lines = []

CSV.foreach(node_ids_csv) do |row|
  name, id, category = row
  next if name.nil? || id.nil?
  const_name = to_upper_snake(name)
  ns0_lines << "    rb_define_const(rb_mNS0, \"#{const_name}\", scada_node_id_numeric(0, #{id}));"
  if category&.strip == "DataType"
    dt_lines << "    rb_define_const(rb_mDataType, \"#{const_name}\", scada_node_id_numeric(0, #{id}));"
  end
end

File.write(File.join(OUT_DIR, "ns0_ids.h"), <<~H)
  #ifndef SCADA_NS0_IDS_H
  #define SCADA_NS0_IDS_H

  #include <ruby.h>

  /* Forward declaration */
  VALUE scada_node_id_numeric(uint16_t ns, uint32_t id);

  static void scada_register_ns0(VALUE rb_mNS0, VALUE rb_mDataType) {
  #{ns0_lines.join("\n")}

  #{dt_lines.join("\n")}
  }

  #endif /* SCADA_NS0_IDS_H */
H

puts "Generated ns0_ids.h (#{ns0_lines.size} NS0 constants, #{dt_lines.size} DataType constants)"

# --- status_codes.h ---
status_csv = File.join(SRC_DIR, "tools/schema/StatusCode.csv")
abort "StatusCode.csv not found at #{status_csv}" unless File.exist?(status_csv)

entries = []
CSV.foreach(status_csv) do |row|
  name, value, description = row
  next if name.nil? || value.nil?
  next if %w[Good Uncertain Bad].include?(name)
  desc = (description || "").gsub('"', '\\"')
  entries << { name: name, value: value, description: desc }
end

var_decls = entries.map { |e| "VALUE eScada#{e[:name]};" }.join("\n")

register_lines = entries.map { |e|
  "    eScada#{e[:name]} = rb_define_class_under(rb_mError, \"#{e[:name]}\", eScadaBase);"
}.join("\n")

switch_cases = entries.map { |e|
  ua_name = "UA_STATUSCODE_#{e[:name].upcase}"
  "        case #{ua_name}:\n            rb_raise(eScada#{e[:name]}, \"%s\", \"#{e[:description]}\");\n            break;"
}.join("\n")

File.write(File.join(OUT_DIR, "status_codes.h"), <<~H)
  #ifndef SCADA_STATUS_CODES_H
  #define SCADA_STATUS_CODES_H

  #include <ruby.h>
  #include "deps/open62541.h"

  VALUE eScadaBase;
  #{var_decls}

  void scada_register_errors(VALUE rb_mError) {
      eScadaBase = rb_define_class_under(rb_mError, "Error", rb_eStandardError);
      rb_gc_register_mark_object(eScadaBase);
  #{register_lines}
  }

  void scada_check_status(UA_StatusCode code) {
      if (code == UA_STATUSCODE_GOOD) return;
      switch (code) {
  #{switch_cases}
          default:
              rb_raise(eScadaBase, "OPC UA error: 0x%08x", code);
      }
  }

  #endif /* SCADA_STATUS_CODES_H */
H

puts "Generated status_codes.h (#{entries.size} error classes)"
