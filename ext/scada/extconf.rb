require "mkmf"

# open62541 amalgamation lives in deps/
deps_dir = File.join(__dir__, "deps")

$srcs = Dir[File.join(__dir__, "*.c")].map { |f| File.basename(f) }
$srcs << "deps/open62541.c"

$INCFLAGS << " -I#{deps_dir}"
$VPATH << "$(srcdir)/deps"

# C99 + open62541 config
$CFLAGS << " -std=c99"
$CFLAGS << " -DUA_ARCHITECTURE_POSIX"
$CFLAGS << " -DUA_NAMESPACE_ZERO=0"

# Try encryption via mbedtls
if have_library("mbedtls") && have_library("mbedx509") && have_library("mbedcrypto")
  $CFLAGS << " -DUA_ENABLE_ENCRYPTION"
  $CFLAGS << " -DUA_ENABLE_ENCRYPTION_MBEDTLS"
else
  warn "*** mbedtls not found, building without encryption support ***"
end

# Allow overriding UA_MAXTIMEOUT (e.g. for faster tests)
if (v = ENV["UA_MAXTIMEOUT"])
  $CFLAGS << " -DUA_MAXTIMEOUT=#{v}"
end

# Catch callback signature mismatches at compile time
$CFLAGS << " -Werror=incompatible-pointer-types"
$CFLAGS << " -Werror=implicit-function-declaration"

# Optimize for size (open62541 amalgamation is large)
$CFLAGS << " -Os"

# Suppress warnings from amalgamation
$warnflags = ""

create_makefile("scada/scada")
