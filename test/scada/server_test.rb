require_relative "../spec_helper"

describe Scada::Server do
  include ScadaHelper

  describe "lifecycle" do
    it "starts and stops cleanly" do
      with_server do |server, port|
        Sync do |task|
          server_task = task.async { server.run }
          sleep 0.05
          server_task.stop
        end
      end
    end
  end

  describe "Config" do
    it "provides sensible defaults" do
      cfg = Scada::Server::Config.default
      assert_equal 4840, cfg.port
      assert_equal "Scada Ruby OPC UA Server", cfg.application_name
      assert_equal Scada::SecurityMode::NONE, cfg.security_mode
    end

    it "allows overriding with .with" do
      cfg = Scada::Server::Config.default.with(port: 5000)
      assert_equal 5000, cfg.port
    end
  end

  describe "#has_node?" do
    it "returns true for added variable" do
      with_server do |server, _port|
        server.add_variable("ns=1;s=temp", type: :double, value: 22.0,
                            display_name: "Temperature")
        assert server.has_node?("ns=1;s=temp")
      end
    end

    it "returns false for nonexistent node" do
      with_server do |server, _port|
        refute server.has_node?("ns=1;s=nonexistent")
      end
    end
  end

  describe "#add_variable" do
    it "adds a variable with initial value" do
      with_server do |server, _port|
        server.add_variable("ns=1;s=setpoint", type: :double, value: 20.0,
                            display_name: "Setpoint")
        assert server.has_node?("ns=1;s=setpoint")
      end
    end
  end

  describe "#add_object" do
    it "adds an object node" do
      with_server do |server, _port|
        server.add_object("ns=1;s=device", display_name: "Device")
        assert server.has_node?("ns=1;s=device")
      end
    end
  end
end
