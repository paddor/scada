# Changelog

## 0.1.0 — 2026-03-12

Initial release.

### Features

- Async client and server built on open62541 v1.5.2 and [async](https://github.com/socketry/async)
- Read, write, and subscribe to OPC UA variables (all built-in scalar types + arrays)
- Data sources with `on_read`/`on_write` callbacks
- Server-side methods with async completion support
- `MethodRequest` passed to callbacks with `input_arguments`, `session_id`, `method_id`, `object_node_id`
- Exception-to-status-code propagation in method callbacks (raise `Scada::Error::BadInvalidArgument`, etc.)
- Data change and event subscriptions with configurable trigger
- mbedTLS encryption — server/client certificates, trust lists, username/password auth
- Development config with auto-generated certificates via [localhost](https://github.com/socketry/localhost)
- Transparent namespace index mapping
- Custom logger support (proc)
- `Server#update_value` for updating variables from server-side code
- Full NS0 node ID constants (`Scada::NS0`) and data type constants (`Scada::DataType`) loaded from bundled CSV
- Typed error subclasses (`Scada::Error::BadTimeout`, etc.) loaded from bundled CSV
- GVL released during `_run_iterate` for both client and server
