# Plan: `scada` — Ruby OPC UA gem wrapping open62541

> Module name: `Scada`.

## Context

Ruby gem providing OPC UA client and server via the open62541 C library. Async client (read/write/call returning `Async::Promise`), async server (variables, objects, methods, data sources, nodeset XML loading), subscriptions (data changes + events), encryption via mbedtls, Ruby Async integration using `run_iterate()` with start-to-start timers. Full namespace zero. All status codes as Ruby exceptions.

## C Extension

Pure C extension via mkmf. open62541 is C99 — no FFI boundary, callbacks are straightforward trampolines, build is simple.

### Build Strategy

`extconf.rb` compiles all `.c` files including the bundled `open62541.c` amalgamation. Flags:
```
-std=c99 -DUA_ARCHITECTURE_POSIX -DUA_NAMESPACE_ZERO=0
  -DUA_ENABLE_ENCRYPTION -DUA_ENABLE_ENCRYPTION_MBEDTLS
```

### What Changes on open62541 Update

`script/update_open62541.sh` clones a pinned tag, builds the amalgamation via cmake, and copies into `deps/`. It also runs code generation: parses `NodeIds.csv` and `StatusCodes.csv` from the open62541 source tree and generates `ns0_ids.h` and `status_codes.h` into `ext/scada/generated/`. The CSVs are transient — only the generated headers are committed.

A Rake task wraps this for convenience:

```ruby
# Rakefile
desc "Update open62541 amalgamation to a given tag (default: latest release)"
task "open62541:update", [:tag] do |_t, args|
  tag = args[:tag] || "v1.5.2"
  sh "script/update_open62541.sh #{tag}"
end
```

Usage: `rake open62541:update` or `rake "open62541:update[v1.5.2]"`

**Ruby version:** Requires Ruby >= 3.2.

Files in `deps/` that change on update:
- `deps/open62541.c` — amalgamated source (~85k lines)
- `deps/open62541.h` — amalgamated header

Generated files that change on update:
- `ext/scada/generated/ns0_ids.h` — NS0 constants + DataType constants
- `ext/scada/generated/status_codes.h` — exception classes + `scada_check_status()`

### NS0 Node IDs + Data Types

Generated from `NodeIds.csv` (columns: `Name,NodeId,Category`):

```ruby
Scada::NS0::OBJECTS_FOLDER           # => #<Scada::NodeId ns=0;i=85>
Scada::NS0::SERVER                   # => #<Scada::NodeId ns=0;i=2253>
Scada::NS0::HAS_COMPONENT           # => #<Scada::NodeId ns=0;i=47>
Scada::NS0::FOLDER_TYPE             # => #<Scada::NodeId ns=0;i=61>

# DataType entries (subset of NS0)
Scada::DataType::BOOLEAN             # => #<Scada::NodeId ns=0;i=1>
Scada::DataType::INT32               # => #<Scada::NodeId ns=0;i=6>
Scada::DataType::DOUBLE              # => #<Scada::NodeId ns=0;i=11>
Scada::DataType::STRING              # => #<Scada::NodeId ns=0;i=12>
```

Symbol shortcuts (`:double`, `:string`, etc.) resolve to `Scada::DataType::*` internally.

### Localized Text

OPC UA `DisplayName` is a `UA_LocalizedText` (locale + text). The Ruby API accepts:
- **String**: treated as text with locale `"en-US"` (default)
- **Hash**: `{ "en-US" => "Temperature", "de-DE" => "Temperatur" }` — first entry used as primary
- Internally stored as `UA_LocalizedText` with the given locale

**Generated `ns0_ids.h`** contains:
```c
static void scada_register_ns0(VALUE rb_mNS0, VALUE rb_mDataType) {
    rb_define_const(rb_mNS0, "OBJECTS_FOLDER", scada_node_id_numeric(0, 85));
    // ... ~1500 more
    rb_define_const(rb_mDataType, "BOOLEAN", scada_node_id_numeric(0, 1));
    // ... ~100 more
}
```

### Error Codes

Generated from `StatusCodes.csv` (columns: `Name,Value,Description`):

```ruby
Scada::Error::Error < StandardError   # base

Scada::Error::BadTimeout              # "The operation timed out."
Scada::Error::BadNodeIdUnknown        # "The node id refers to a node that does not exist..."

begin
  client.read(ns["nonexistent"]).wait
rescue Scada::Error::BadNodeIdUnknown => e
  puts e.message
rescue Scada::Error::Error => e
  # catch-all
end
```

**Generated `status_codes.h`** contains:
```c
static VALUE eScadaBadTimeout;
static VALUE eScadaBadNodeIdUnknown;
// ... ~200 more

static void scada_register_errors(VALUE rb_mError) {
    VALUE base = rb_define_class_under(rb_mError, "Error", rb_eStandardError);
    eScadaBadTimeout = rb_define_class_under(rb_mError, "BadTimeout", base);
    // ...
}

static void scada_check_status(UA_StatusCode code) {
    if (code == UA_STATUSCODE_GOOD) return;
    switch (code) {
        case UA_STATUSCODE_BADTIMEOUT:
            rb_raise(eScadaBadTimeout, "The operation timed out.");
            break;
        // ... ~200 more
        default:
            rb_raise(eScadaBase, "OPC UA error: 0x%08x", code);
    }
}
```

### Callback Architecture

During `_run_iterate` (GVL released), open62541 fires our C callbacks:

```
Ruby: server.run -> _run_iterate
  C: rb_thread_call_without_gvl(server_iterate_nogvl, ...)
    open62541 fires data_source_read_callback (our trampoline)
      C: rb_thread_call_with_gvl(invoke_proc_with_gvl, ...)
        Ruby: user's Proc executes (GVL held)
      C: GVL released again, return to open62541
    open62541 returns
  C: GVL re-acquired, return to Ruby
Ruby: sleep, repeat
```

Callback context struct stored as `void* nodeContext`:
```c
typedef struct {
    VALUE read_proc;    // Ruby Proc (GC-marked)
    VALUE write_proc;   // Ruby Proc (GC-marked)
} ScadaDataSourceCtx;
```

Procs kept alive via a Ruby Array on the Server/Client object, marked in the `dmark` function.

### Memory Management

TypedData API (`rb_data_type_t`) with `dmark`, `dfree`, `dsize`:

```c
typedef struct {
    UA_Server *server;
    VALUE callback_procs;  // Ruby Array — prevents GC of stored Procs
} ScadaServer;

static void server_mark(void *ptr) {
    ScadaServer *s = ptr;
    rb_gc_mark(s->callback_procs);
}

static void server_free(void *ptr) {
    ScadaServer *s = ptr;
    if (s->server) UA_Server_delete(s->server);
    xfree(s);
}

static const rb_data_type_t server_type = {
    .wrap_struct_name = "Scada::Server",
    .function = { .dmark = server_mark, .dfree = server_free, .dsize = server_memsize },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};
```

## Gem Structure

```
scada/
  scada.gemspec
  Gemfile
  Rakefile                # compile, spec, open62541:update tasks
  ext/scada/
    extconf.rb
    deps/
      open62541.c           # bundled amalgamation (changes on update)
      open62541.h           # bundled header (changes on update)
    generated/
      ns0_ids.h             # pre-generated by update script
      status_codes.h        # pre-generated by update script
    scada.c                 # Init_scada(), module/class hierarchy
    scada.h                 # shared declarations, extern VALUEs
    node_id.c / node_id.h
    variant.c / variant.h
    status.c / status.h
    server.c / server.h
    client.c / client.h
    callbacks.c / callbacks.h
  lib/
    scada.rb                # loads native ext + ruby wrappers
    scada/
      version.rb
      data_value.rb         # Data.define(:value, :status, :source_timestamp, :server_timestamp)
      status.rb             # Data.define(:code) with #good?, #bad?, #to_s, #message
      event.rb              # Data.define(:event_type, :message, :severity, :source_name, :time)
      node_info.rb          # Data.define(:node_id, :node_class, :display_name, :browse_name, :parent, :organizes)
      node_id.rb            # convenience parsing ("ns=1;s=foo")
      security_mode.rb      # module with NONE, SIGN, SIGN_AND_ENCRYPT
      log_category.rb       # module with NETWORK, SECURECHANNEL, etc.
      server.rb             # Async-aware #run, add_variable/add_method/add_object, Config
      client.rb             # Async-aware #run, async read/write/call, Config
      client/
        subscription.rb     # Subscription wrapper
        monitored_item.rb   # MonitoredItem wrapper
        namespace.rb        # Namespace resolver
  spec/
    spec_helper.rb
    scada_helper.rb         # with_server_and_client, random_port
    node_id_spec.rb
    status_spec.rb
    server_spec.rb
    client_spec.rb
    variable_spec.rb
    data_source_spec.rb
    method_spec.rb
    subscription_spec.rb
    event_subscription_spec.rb
    encryption_spec.rb
    namespace_spec.rb
  examples/
  script/
    update_open62541.sh     # clones repo, cmake, copies deps + generates headers
```

## Value Objects (Data.define)

```ruby
Scada::DataValue = Data.define(:value, :status, :source_timestamp, :server_timestamp)

Scada::Status = Data.define(:code) do
  def good? = (code & 0xC0000000) == 0
  def bad?  = (code & 0x80000000) != 0
  def to_s  = # short name from generated table
  def message = # long description from generated table
  def inspect = "#<Scada::Status 0x#{code.to_s(16).rjust(8, '0')} #{to_s}>"
end

Scada::Event = Data.define(:event_type, :message, :severity, :source_name, :time)

Scada::NodeInfo = Data.define(
  :node_id, :node_class, :display_name, :browse_name,
  :parent, :organizes
)
```

## Modules

```ruby
module Scada::SecurityMode
  NONE             = :none
  SIGN             = :sign
  SIGN_AND_ENCRYPT = :sign_and_encrypt
end

module Scada::LogCategory
  NETWORK        = :network
  SECURECHANNEL  = :securechannel
  SESSION        = :session
  SERVER         = :server
  CLIENT         = :client
  USERLAND       = :userland
  SECURITYPOLICY = :securitypolicy
  EVENTLOOP      = :eventloop
  PUBSUB         = :pubsub
  DISCOVERY      = :discovery
end
```

## Config Objects (Data.define)

```ruby
# @!attribute port [Integer] TCP port (default: 4840)
# @!attribute application_name [String] OPC UA application name
# @!attribute application_uri [String] OPC UA application URI
# @!attribute product_uri [String] OPC UA product URI
# @!attribute certificate [String, Pathname, OpenSSL::X509::Certificate, nil] Server certificate
# @!attribute private_key [String, Pathname, OpenSSL::PKey, nil] Server private key
# @!attribute trust_list [Array<String, Pathname, OpenSSL::X509::Certificate>] Trusted client certificates
# @!attribute security_mode [Symbol] One of SecurityMode constants
# @!attribute users [Hash{String => String}] username => password pairs
# @!attribute allow_anonymous [Boolean, nil] Allow anonymous logins
# @!attribute sampling_interval [Range<Float>] Supported sampling interval range (seconds)
# @!attribute publishing_interval [Range<Float>] Supported publishing interval range (seconds)
# @!attribute logger [#call, nil] Logger proc receiving (category, level, message)
class Scada::Server::Config < Data.define(
  :port, :application_name, :application_uri, :product_uri,
  :certificate, :private_key, :trust_list,
  :security_mode, :users, :allow_anonymous,
  :sampling_interval, :publishing_interval,
  :logger
)
  def self.default
    new(
      port: 4840,
      application_name: "Scada Ruby OPC UA Server",
      application_uri: "urn:scada:server",
      product_uri: "urn:scada.rb",
      certificate: nil, private_key: nil, trust_list: [],
      security_mode: SecurityMode::NONE,
      users: {}, allow_anonymous: nil,
      sampling_interval: 0.005..10.0,
      publishing_interval: 0.01..60.0,
      logger: nil
    )
  end

  def self.secure(certificate:, private_key:, **opts)
    default.with(
      certificate: certificate, private_key: private_key,
      security_mode: SecurityMode::SIGN_AND_ENCRYPT,
      **opts
    )
  end

  def self.development
    authority = Localhost::Authority.fetch
    cert = authority.server_identity.certificate
    key = authority.server_identity.key
    default.with(
      certificate: cert, private_key: key,
      security_mode: SecurityMode::SIGN_AND_ENCRYPT,
      application_uri: cert.extensions.find { |e|
        e.oid == "subjectAltName"
      }&.value&.[](/URI:(.+)/, 1) || "urn:scada:server"
    )
  end
end

# @!attribute application_name [String] OPC UA application name
# @!attribute application_uri [String] OPC UA application URI
# @!attribute product_uri [String] OPC UA product URI
# @!attribute certificate [String, Pathname, OpenSSL::PKey, nil] Client certificate
# @!attribute private_key [String, Pathname, OpenSSL::PKey, nil] Client private key
# @!attribute trust_list [Array<String, Pathname, OpenSSL::X509::Certificate>] Trusted server certificates
# @!attribute security_mode [Symbol] One of SecurityMode constants
# @!attribute username [String, nil] Username for authentication
# @!attribute password [String, nil] Password for authentication
# @!attribute logger [#call, nil] Logger proc
class Scada::Client::Config < Data.define(
  :application_name, :application_uri, :product_uri,
  :certificate, :private_key, :trust_list,
  :security_mode, :username, :password,
  :logger
)
  def self.default
    new(
      application_name: "Scada Ruby OPC UA Client",
      application_uri: "urn:scada:client",
      product_uri: "urn:scada.rb",
      certificate: nil, private_key: nil, trust_list: [],
      security_mode: SecurityMode::NONE,
      username: nil, password: nil,
      logger: nil
    )
  end

  def self.secure(certificate:, private_key:, **opts)
    default.with(
      certificate: certificate, private_key: private_key,
      security_mode: SecurityMode::SIGN_AND_ENCRYPT,
      **opts
    )
  end

  def self.development
    authority = Localhost::Authority.fetch
    cert = authority.client_identity.certificate
    key = authority.client_identity.key
    default.with(certificate: cert, private_key: key,
                 security_mode: SecurityMode::SIGN_AND_ENCRYPT)
  end
end
```

Certificate/key input handling:

| Input | Behavior |
|-------|----------|
| `String` | Raw certificate bytes (DER or PEM) |
| `Pathname` | Reads file at path |
| `OpenSSL::X509::Certificate` | Calls `#to_der` |
| `OpenSSL::PKey::RSA` / `PKey::EC` | Calls `#to_der` |

## Ruby API Design

### Server

```ruby
server = Scada::Server.new(config: Scada::Server::Config.default)

# Internal store variable with optional write validation
# display_name accepts String (defaults to locale "en-US") or Hash for localized text
server.add_variable("ns=1;s=setpoint", type: :double, value: 20.0,
  display_name: "Setpoint", browse_name: "Setpoint") do |old_dv, new_dv|
  raise "Out of range" unless (10.0..50.0).include?(new_dv.value)
end

# Localized display name
server.add_variable("ns=1;s=sensor", type: :double,
  display_name: { "en-US" => "Sensor", "de-DE" => "Sensor" },
  on_read: -> { read_hardware() },
  on_write: ->(old_dv, new_dv) { write_hardware(new_dv.value) })

# Object node
server.add_object("ns=1;s=device",
  display_name: "Device", parent: Scada::NS0::OBJECTS_FOLDER)

# Method — all methods are async internally (UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY).
# Block receives a single request object. Three completion modes:
#
# 1. Return value auto-committed (request.commit not called, no #wait):
server.add_method("ns=1;s=greet",
  display_name: "Greet",
  input:  [{ name: "name", type: :string }],
  output: [{ name: "greeting", type: :string }]) { |req| "Hello, #{req.input[0]}!" }
#
# 2. Async: return responds to #wait — committed when task resolves,
#    error status on exception:
server.add_method("ns=1;s=provision",
  display_name: "Provision",
  input:  [{ name: "device_id", type: :string }],
  output: [{ name: "status", type: :string }]) do |req|
  Async { provision_device(req.input[0]) }
end
#
# 3. Manual commit with custom status:
server.add_method("ns=1;s=validate",
  display_name: "Validate",
  input:  [{ name: "value", type: :double }],
  output: [{ name: "ok", type: :boolean }]) do |req|
  if req.input[0] < 0
    req.status = 0x8016_0000  # BadOutOfRange
    req.commit(false)
  else
    req.commit(true)
  end
end

# Nodeset XML loading with callback binding
server.load_nodeset("nodeset/mydevice.xml")
ns = server.namespace("urn:mycompany:mydevice")
server.on_read(ns["sensor"]) { read_from_hardware() }
server.on_write(ns["setpoint"]) { |old_dv, new_dv| ... }
server.on_call(ns["reset"]) { |req| do_reset(*req.input) }
server.on_call(ns["firmware_update"]) { |req| Async { update(req.input[0]) } }

# DSL block form
server.bind(ns) do
  on_read("sensor") { read_from_hardware() }
  on_write("setpoint") { |old_dv, new_dv| ... }
end

# Node introspection
server.has_node?(ns["setpoint"])
server.each_node(namespace: ns, node_class: :variable) { |node| ... }
```

### Client

```ruby
client = Scada::Client.new("opc.tcp://localhost:4840",
  config: Scada::Client::Config.default)
client.connect

# Namespace resolution
ns = client.namespace("urn:mycompany:device")

# Async read/write/call — all return Async::Promise
dv = client.read(ns["temp"]).wait                           # single
dvs = client.read(ns["temp"], ns["pressure"]).wait          # batch
client.read(ns["temp"], attribute: :display_name).wait      # other attribute
client.write(ns["temp"], 25.0).wait
result = client.call(ns["greet"], "World").wait

# Subscriptions — data changes
sub = client.subscribe(publish_interval: 0.1)
sub.monitor_data_changes(ns["temp"]) { |dv| ... }  # primary name
sub.monitor(ns["temp"]) { |dv| ... }                # alias

# Subscriptions — events
sub.monitor_events(Scada::NS0::SERVER,
  select: [:event_type, :message, :severity, :time],
  where: { severity: { gte: 500 } }) { |event| ... }
```

### Async Integration

```ruby
Async do |task|
  task.async { server.run }
  task.async { client.run }
end
```

**Server/Client event loop** (start-to-start timer, relies on `Async::Task#stop`):

```ruby
module Scada
  class Server
    TICK = 0.02

    def run
      _run_startup
      run_next = Async::Clock.now
      loop do
        run_next += TICK
        _run_iterate
        remaining = run_next - Async::Clock.now
        if remaining > 0
          sleep(remaining)
        else
          Async::Task.current.yield
        end
      end
    ensure
      _run_shutdown
    end
  end
end
```

**C side**: `#_run_iterate` releases GVL via `rb_thread_call_without_gvl`.

## NodeId Types

All four OPC UA node ID types supported:

```ruby
Scada::NodeId.parse("ns=1;i=42")           # numeric
Scada::NodeId.parse("ns=1;s=temperature")  # string
Scada::NodeId.parse("ns=1;g=...")          # guid
Scada::NodeId.parse("ns=1;b=...")          # opaque (bytestring)
```

## Type Mapping (variant.c)

| OPC UA | Ruby | Symbol |
|--------|------|--------|
| Boolean | true/false | `:boolean` |
| Int32 | Integer | `:int32` |
| UInt32 | Integer | `:uint32` |
| Int64 | Integer | `:int64` |
| Double | Float | `:double` |
| Float | Float | `:float` |
| String | String | `:string` |
| DateTime | Time | `:datetime` |
| ByteString | String (binary) | `:byte_string` |
| NodeId | Scada::NodeId | `:node_id` |
| Arrays | Ruby Array | (inferred) |

## Specs (minitest/spec)

Use `assert_*` / `refute_*`. Each test gets isolated server/client instances via `ScadaHelper#with_server_and_client`.

### Test Helper

```ruby
module ScadaHelper
  def with_server_and_client(config: nil, **server_opts)
    port = random_port
    cfg = (config || Scada::Server::Config.default).with(port: port)
    server = Scada::Server.new(config: cfg, **server_opts)
    yield server if block_given?
    Async do |task|
      server_task = task.async { server.run }
      client = Scada::Client.new("opc.tcp://localhost:#{port}")
      client.connect
      client_task = task.async { client.run }
      @server, @client = server, client
      @ns = client.namespace("urn:test")
      yield_to_test
    ensure
      client_task&.stop
      client&.disconnect
      server_task&.stop
    end
  end

  private

  def random_port
    s = TCPServer.new("127.0.0.1", 0)
    s.addr[1]
  ensure
    s&.close
  end
end
```

### Spec List

1. **node_id_spec.rb** — parsing all 4 types (i, s, g, b), equality, `#to_s`, invalid input
2. **status_spec.rb** — `#good?`, `#bad?`, `#to_s`, `#message`, `#inspect`
3. **server_spec.rb** — server lifecycle, default port, custom config, `#has_node?`, `#each_node`
4. **client_spec.rb** — connect, disconnect, namespace resolution, read single/batch, read other attributes, write, call, promise rejection on bad node
5. **variable_spec.rb** — initial value, write + read back, on_write callback with old/new DataValue, write rollback (raise in callback)
6. **data_source_spec.rb** — on_read callback, on_write callback, no internal store
7. **method_spec.rb** — simple method (return value auto-committed), async method (returns Async::Task, committed on resolve), async method error (task raises → error status), manual commit with custom status code
8. **subscription_spec.rb** — `#monitor_data_changes` fires on value change, publish interval
9. **event_subscription_spec.rb** — `#monitor_events` with select/where, event fields
10. **namespace_spec.rb** — `client.namespace(uri)`, `ns["name"]` resolution
11. **encryption_spec.rb**:
    - No encryption: server + client with default configs connect and communicate
    - Development: `Server::Config.development` + `Client::Config.development` using localhost.rb
    - Production: `Server::Config.secure(cert, key, trust_list: [...])` + `Client::Config.secure(cert, key, trust_list: [...])`
    - Rejected connection: encrypted server rejects unencrypted client
    - Username/password auth: server with `users: {"admin" => "pass"}`, client with matching credentials
    - Bad credentials: client with wrong password gets rejected

### Encryption Specs Detail

```ruby
describe "encryption" do
  describe "no encryption" do
    it "connects and communicates with default configs" do
      server = Scada::Server.new(config: Scada::Server::Config.default.with(port: random_port))
      server.add_variable("ns=1;s=temp", type: :double, value: 22.0)

      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}")
        client.connect
        task.async { client.run }

        dv = client.read("ns=1;s=temp").wait
        assert_in_delta 22.0, dv.value, 0.01
      ensure
        # cleanup
      end
    end
  end

  describe "development encryption" do
    it "connects using localhost.rb certificates" do
      server_config = Scada::Server::Config.development.with(port: random_port)
      client_config = Scada::Client::Config.development
      server = Scada::Server.new(config: server_config)
      server.add_variable("ns=1;s=temp", type: :double, value: 22.0)

      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}", config: client_config)
        client.connect
        task.async { client.run }

        dv = client.read("ns=1;s=temp").wait
        assert_in_delta 22.0, dv.value, 0.01
      ensure
        # cleanup
      end
    end
  end

  describe "production encryption" do
    it "connects with explicit certificates and trust lists" do
      server_cert, server_key = generate_test_cert("server")
      client_cert, client_key = generate_test_cert("client")

      server_config = Scada::Server::Config.secure(
        certificate: server_cert, private_key: server_key,
        trust_list: [client_cert], port: random_port
      )
      client_config = Scada::Client::Config.secure(
        certificate: client_cert, private_key: client_key,
        trust_list: [server_cert]
      )

      server = Scada::Server.new(config: server_config)
      server.add_variable("ns=1;s=temp", type: :double, value: 22.0)

      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}", config: client_config)
        client.connect
        task.async { client.run }

        dv = client.read("ns=1;s=temp").wait
        assert_in_delta 22.0, dv.value, 0.01
      ensure
        # cleanup
      end
    end

    it "rejects unencrypted client connecting to encrypted server" do
      server_cert, server_key = generate_test_cert("server")
      server_config = Scada::Server::Config.secure(
        certificate: server_cert, private_key: server_key,
        port: random_port, allow_anonymous: false
      )

      server = Scada::Server.new(config: server_config)
      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}")
        assert_raises(Scada::Error::Error) { client.connect }
      ensure
        # cleanup
      end
    end

    it "authenticates with username and password" do
      server_config = Scada::Server::Config.default.with(
        port: random_port,
        users: { "admin" => "secret" },
        allow_anonymous: false
      )
      client_config = Scada::Client::Config.default.with(
        username: "admin", password: "secret"
      )

      server = Scada::Server.new(config: server_config)
      server.add_variable("ns=1;s=temp", type: :double, value: 22.0)

      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}", config: client_config)
        client.connect
        task.async { client.run }

        dv = client.read("ns=1;s=temp").wait
        assert_in_delta 22.0, dv.value, 0.01
      ensure
        # cleanup
      end
    end

    it "rejects bad credentials" do
      server_config = Scada::Server::Config.default.with(
        port: random_port,
        users: { "admin" => "secret" },
        allow_anonymous: false
      )
      client_config = Scada::Client::Config.default.with(
        username: "admin", password: "wrong"
      )

      server = Scada::Server.new(config: server_config)
      Async do |task|
        task.async { server.run }
        client = Scada::Client.new("opc.tcp://localhost:#{port}", config: client_config)
        assert_raises(Scada::Error::Error) { client.connect }
      ensure
        # cleanup
      end
    end
  end
end
```

## Implementation Phases

### Phase 1: Foundation
- Gem skeleton: gemspec, Gemfile, Rakefile, extconf.rb
- Bundle open62541 amalgamation in `deps/`
- Pre-generated `ns0_ids.h` and `status_codes.h` in `generated/`
- `scada.c` — `Init_scada()`, module hierarchy (Scada, NS0, DataType, Error)
- `status.c` — includes generated header, `scada_check_status()`
- `node_id.c` — NodeId wrapping (TypedData), parsing (i/s/g/b), `#==`, `#to_s`
- `variant.c` — core type conversions (Boolean, Int32, Double, String)
- `server.c` — new (with Config), `_run_startup`, `_run_iterate`, `_run_shutdown`, `_add_variable_node`
- `client.c` — new (with Config), connect, disconnect, `_read`, `_write`
- Ruby wrappers: DataValue, Status, server.rb, client.rb, node_id.rb
- First spec: server + client read/write a variable

### Phase 2: GVL + Async
- `rb_thread_call_without_gvl` in `_run_iterate` for both server and client
- Start-to-start timer in Ruby `#run` using `Async::Clock`
- `Async::Task.current.yield` when not sleeping
- `Async::Promise` for client `#read`, `#write`
- Namespace resolution: `Client#namespace(uri)` using `UA_Client_NamespaceGetIndex`
- Specs: Async server + client in separate fibers, promise resolution

### Phase 3: Methods + Objects
- `callbacks.c` — method callback trampoline (GVL re-entry via `rb_thread_call_with_gvl`)
- `server.c` — `_add_method_node`, `_add_object_node`
- All methods async internally: `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY` + `UA_Server_setAsyncCallMethodResult`
- Block receives single `request` object (`req.input` for input args); if `request.commit` called → done; elif return responds to `#wait` → committed on resolve (error on exception); else → auto-commit return value
- `request.status=` allows setting custom UA status codes before `request.commit`
- `client.c` — `_call`
- Ruby wrappers: `#add_method`, `#add_object`, `#call`
- Specs: simple method, deferred method, object creation

### Phase 4: Data Sources + Write Validation
- Data source read/write trampolines in `callbacks.c`
- GC marking for callback Procs
- Write validation: internal store variables with block callback receiving old/new DataValue
- Rollback: raise in callback prevents write
- `#add_variable` unified API: `value:` vs `on_read:`/`on_write:`
- Specs: data source read/write, write validation, rollback

### Phase 5: Subscriptions
- `client.c` — subscription create, monitored item add (data changes + events)
- `callbacks.c` — data change notification trampoline, event notification trampoline
- Ruby: `Client#subscribe`, `Subscription#monitor_data_changes` (alias `#monitor`), `Subscription#monitor_events`
- Event Data.define
- Specs: data change subscription, event subscription

### Phase 6: Nodeset XML + Node Introspection
- `server.c` — `_load_nodeset`
- `#on_read`, `#on_write`, `#on_call`, `#on_async_call` for binding callbacks to existing nodes
- `#bind(ns)` DSL block
- `#has_node?`, `#each_node`
- NodeInfo Data.define
- Specs: load XML, bind callbacks, node introspection

### Phase 7: Encryption
- Build flag: `-DUA_ENABLE_ENCRYPTION -DUA_ENABLE_ENCRYPTION_MBEDTLS`
- `server.c` — configure security policies from Config (certificate, key, trust list, users)
- `client.c` — configure security from Config (certificate, key, trust list, credentials)
- Certificate input normalization: String (bytes), Pathname (file), OpenSSL objects (#to_der)
- `Server::Config.secure`, `Server::Config.development`
- `Client::Config.secure`, `Client::Config.development`
- SecurityMode validation (raise on unknown)
- Specs: no encryption, development (localhost.rb), production, rejected connections, username/password auth

### Phase 8: Polish
- Remaining variant types (arrays, DateTime, ByteString, UInt32, Int64, Float)
- Custom logger support
- LogCategory module
- Memory leak testing
- `script/update_open62541.sh`
- Examples

## Verification

1. `rake compile` — extension builds cleanly
2. `rake spec` — all tests pass
3. `Scada::NS0.constants.length` returns ~1500
4. `Scada::DataType.constants` lists all OPC UA data types
5. `rescue Scada::Error::BadTimeout => e; e.message` returns human-readable text
6. Async server + client communicate in fibers
7. `client.read(ns["temp"]).wait` returns DataValue with correct value
8. Data source callbacks fire on read/write
9. Write validation blocks invalid writes
10. Subscriptions fire on data change and events
11. Encrypted server + client communicate with localhost.rb
12. Unencrypted client rejected by encrypted server
13. Memory: no leaks over repeated create/destroy cycles
14. Signal: Ctrl-C cleanly shuts down via `Async::Task#stop`

## Reference Patterns
- TypedData API (`rb_data_type_t`) with `dmark`, `dfree`, `dsize` for wrapping C structs
- `rb_thread_call_without_gvl` for releasing GVL during blocking C calls
- `rb_gc_register_mark_object` for protecting static VALUE exception classes from GC
- Argument struct pattern: define struct on stack, pass to nogvl callback, read results after
