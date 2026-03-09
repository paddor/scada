# Scada

OPC UA client and server for Ruby, built on [open62541](https://www.open62541.org/) v1.5.2.

Fully async via [async](https://github.com/socketry/async) — reads, writes, method calls, and subscriptions all run without blocking.

## Installation

```ruby
gem "scada"
```

Requires Ruby >= 3.2. The open62541 amalgamation is bundled — no system libraries needed. mbedTLS is included for encryption support.

## Quick Start

### Server

```ruby
require "scada"

Sync do |task|
  server = Scada::Server.new(config: Scada::Server::Config.default.with(port: 4840))

  server.add_variable("ns=1;s=temperature", type: :double, value: 20.0,
                      display_name: "Temperature")
  server.write_value("ns=1;s=temperature", 20.0, type: :double)

  server.add_method("ns=1;s=greet",
    display_name: "Greet",
    input:  [{ name: "name", type: :string }],
    output: [{ name: "greeting", type: :string }]) { |name| "Hello, #{name}!" }

  task.async { server.run }

  # Update values from your application logic
  loop do
    sleep 1
    server.write_value("ns=1;s=temperature", rand(18.0..25.0), type: :double)
  end
end
```

### Client

```ruby
require "scada"

Sync do |task|
  client = Scada::Client.new("opc.tcp://localhost:4840")
  client.connect
  task.async { client.run }

  # Read
  dv = client.read("ns=1;s=temperature").wait
  puts "Temperature: #{dv.value}°C"

  # Write
  client.write("ns=1;s=temperature", 22.5).wait

  # Write with explicit type (required for non-default types)
  client.write("ns=1;s=sensor_id", 42, type: :uint16).wait

  # Call a method
  result = client.call("ns=1;s=greet", "World").wait
  puts result # => "Hello, World!"

  # Subscribe to changes
  sub = client.subscribe(publish_interval: 0.1)
  sub.monitor_data_changes("ns=1;s=temperature") do |dv|
    puts "New value: #{dv.value}"
  end

  sleep 10
end
```

## Features

### Variable Types

All OPC UA built-in scalar types are supported for read and write:

| OPC UA Type | Ruby Type | Symbol |
|---|---|---|
| Boolean | `true`/`false` | `:boolean` |
| SByte | Integer | `:sbyte` |
| Byte | Integer | `:byte` |
| Int16 | Integer | `:int16` |
| UInt16 | Integer | `:uint16` |
| Int32 | Integer | `:int32` |
| UInt32 | Integer | `:uint32` |
| Int64 | Integer | `:int64` |
| UInt64 | Integer | `:uint64` |
| Float | Float | `:float` |
| Double | Float | `:double` |
| String | String | `:string` |
| DateTime | Time | `:datetime` |
| ByteString | String (ASCII-8BIT) | `:byte_string` |
| NodeId | `Scada::NodeId` | `:node_id` |

Arrays of any type are also supported.

### Data Sources

For variables backed by application logic instead of stored values:

```ruby
server.add_variable("ns=1;s=cpu_load", type: :double,
  display_name: "CPU Load",
  on_read:  -> { `cat /proc/loadavg`.split.first.to_f },
  on_write: ->(val) { puts "Client wrote: #{val}" })
```

### Subscriptions

```ruby
sub = client.subscribe(publish_interval: 0.1)

# Data change monitoring with trigger control
sub.monitor_data_changes("ns=1;s=temp",
  trigger: :status_value_timestamp) { |dv| puts dv.value }

# Event monitoring
sub.monitor_events("i=2253",
  select: ["EventType", "Message", "Severity"]) do |event|
  puts "#{event.message} (severity: #{event.severity})"
end
```

Available triggers: `:status`, `:status_value` (default), `:status_value_timestamp`.

### Encryption

```ruby
# Server with TLS
server = Scada::Server.new(config: Scada::Server::Config.secure(
  certificate: OpenSSL::X509::Certificate.new(File.read("server_cert.pem")),
  private_key: OpenSSL::PKey::RSA.new(File.read("server_key.pem")),
  users: { "admin" => "secret" }
))

# Client with TLS + credentials
client = Scada::Client.new("opc.tcp://localhost:4840",
  config: Scada::Client::Config.secure(
    certificate: OpenSSL::X509::Certificate.new(File.read("client_cert.pem")),
    private_key: OpenSSL::PKey::RSA.new(File.read("client_key.pem")),
    username: "admin", password: "secret"
  ))
```

For development, `Config.development` auto-generates certificates via the [localhost](https://github.com/socketry/localhost) gem.

### Namespaces

```ruby
# Server
ns_idx = server.add_namespace("urn:myapp:sensors")

# Client — transparent namespace index mapping (v1.5 feature)
idx = client.get_namespace_index("urn:myapp:sensors")
```

### Custom Logger

```ruby
Scada::Server.new(config: Scada::Server::Config.default.with(
  logger: ->(category, level, message) {
    puts "[#{level}] #{category}: #{message}"
  }
))

# Suppress all logging
Scada::Server.new(config: Scada::Server::Config.default.with(logger: :silent))
```

### Server-Side Value Updates

Use `write_value` to update a variable with an explicit `sourceTimestamp`:

```ruby
server.write_value("ns=1;s=temperature", 22.5, type: :double)
```

This pins the `sourceTimestamp`, which is important for subscriptions using the `status_value_timestamp` trigger — without it, older open62541 versions would generate a new timestamp on every read, flooding subscribers with spurious notifications.

## Version Info

```ruby
Scada::VERSION           # => "0.1.0"
Scada::OPEN62541_VERSION # => "v1.5.2"
```

## License

MIT
