require "rake/extensiontask"
require "rake/testtask"

Rake::ExtensionTask.new("scada") do |ext|
  ext.lib_dir = "lib/scada"
end

Rake::TestTask.new(:test) do |t|
  t.libs << "test"
  t.pattern = "test/**/*_test.rb"
end

desc "Compile with fast shutdown and run tests"
task fast_test: [:fast_compile, :test]

task :fast_compile do
  ENV["UA_MAXTIMEOUT"] = "1"
  Rake::Task[:compile].invoke
end

task default: [:compile, :test]

desc "Update open62541 amalgamation to a given tag (default: latest release)"
task "open62541:update", [:tag] do |_t, args|
  tag = args[:tag] || "v1.5.2"
  sh "script/update_open62541.sh #{tag}"
end
