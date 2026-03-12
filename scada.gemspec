require_relative "lib/scada/version"

Gem::Specification.new do |s|
  s.name        = "scada"
  s.version     = Scada::VERSION
  s.summary     = "OPC UA client and server for Ruby"
  s.description = "Ruby gem providing OPC UA client and server via the open62541 C library. Async client/server, subscriptions, encryption."
  s.authors     = ["roadster"]
  s.license     = "MIT"
  s.required_ruby_version = ">= 3.2"
  s.homepage    = "https://github.com/paddor/scada"
  s.metadata    = {
    "rubygems_mfa_required" => "true",
    "source_code_uri"       => "https://github.com/paddor/scada",
    "changelog_uri"         => "https://github.com/paddor/scada/blob/main/CHANGELOG.md",
    "bug_tracker_uri"       => "https://github.com/paddor/scada/issues",
  }

  s.files = Dir["lib/**/*.rb", "ext/**/*.{c,h,rb}", "data/**/*", "LICENSE", "README.md"]
  s.extensions = ["ext/scada/extconf.rb"]
  s.require_paths = ["lib"]

  s.add_dependency "async", "~> 2.0"
  s.add_development_dependency "rake-compiler", "~> 1.2"
  s.add_development_dependency "minitest", "~> 6.0"
  s.add_development_dependency "localhost", "~> 1.0"
end
