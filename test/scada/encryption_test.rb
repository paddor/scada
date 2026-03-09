require_relative "../spec_helper"
require "openssl"

module EncryptionHelper
  def generate_opcua_cert(cn:, san_uri:)
    key = OpenSSL::PKey::RSA.new(2048)
    cert = OpenSSL::X509::Certificate.new
    cert.version = 2
    cert.serial = rand(1..0xFFFFFFFF)
    cert.subject = OpenSSL::X509::Name.new([["CN", cn]])
    cert.issuer = cert.subject
    cert.not_before = Time.now - 3600
    cert.not_after = Time.now + 3600 * 24 * 365
    cert.public_key = key.public_key

    ef = OpenSSL::X509::ExtensionFactory.new
    ef.subject_certificate = cert
    ef.issuer_certificate = cert
    cert.add_extension(ef.create_extension("subjectAltName", "URI:#{san_uri}", false))
    cert.add_extension(ef.create_extension("basicConstraints", "CA:FALSE", true))
    cert.add_extension(ef.create_extension("keyUsage",
      "digitalSignature,nonRepudiation,keyEncipherment,dataEncipherment", true))
    cert.add_extension(ef.create_extension("extendedKeyUsage", "serverAuth,clientAuth", false))

    cert.sign(key, OpenSSL::Digest::SHA256.new)
    [cert, key]
  end
end

describe "Encryption" do
  include ScadaHelper
  include EncryptionHelper

  it "connects with encryption using OpenSSL objects" do
    server_cert, server_key = generate_opcua_cert(
      cn: "Test Server", san_uri: "urn:scada:test:server")
    client_cert, client_key = generate_opcua_cert(
      cn: "Test Client", san_uri: "urn:scada:test:client")

    server_config = Scada::Server::Config.secure(
      certificate: server_cert, private_key: server_key,
      trust_list: [client_cert]
    ).with(application_uri: "urn:scada:test:server")

    with_server(config: server_config) do |server, port|
      server.add_variable("ns=1;s=secure_var", type: :double, value: 42.0,
                          display_name: "Secure Variable")

      Sync do |task|
        server_task = task.async { server.run }

        client_config = Scada::Client::Config.secure(
          certificate: client_cert, private_key: client_key,
          trust_list: [server_cert]
        ).with(application_uri: "urn:scada:test:client")

        client = new_client("opc.tcp://localhost:#{port}",
                            config: client_config)
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=secure_var").wait
        assert_instance_of Scada::DataValue, dv
        assert_in_delta 42.0, dv.value, 0.001
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "connects with encryption using DER strings" do
    server_cert, server_key = generate_opcua_cert(
      cn: "Test Server", san_uri: "urn:scada:test:server")
    client_cert, client_key = generate_opcua_cert(
      cn: "Test Client", san_uri: "urn:scada:test:client")

    server_config = Scada::Server::Config.secure(
      certificate: server_cert.to_der, private_key: server_key.to_der,
      trust_list: [client_cert.to_der]
    ).with(application_uri: "urn:scada:test:server")

    with_server(config: server_config) do |server, port|
      server.add_variable("ns=1;s=secure_var", type: :double, value: 99.0,
                          display_name: "Secure Variable")

      Sync do |task|
        server_task = task.async { server.run }

        client_config = Scada::Client::Config.secure(
          certificate: client_cert.to_der, private_key: client_key.to_der,
          trust_list: [server_cert.to_der]
        ).with(application_uri: "urn:scada:test:client")

        client = new_client("opc.tcp://localhost:#{port}",
                            config: client_config)
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=secure_var").wait
        assert_instance_of Scada::DataValue, dv
        assert_in_delta 99.0, dv.value, 0.001
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "connects with username and password" do
    server_config = Scada::Server::Config.default.with(
      users: { "admin" => "secret" }, allow_anonymous: false
    )

    with_server(config: server_config) do |server, port|
      server.add_variable("ns=1;s=auth_var", type: :int32, value: 7,
                          display_name: "Auth Variable")

      Sync do |task|
        server_task = task.async { server.run }

        client_config = Scada::Client::Config.default.with(
          username: "admin", password: "secret"
        )
        client = new_client("opc.tcp://localhost:#{port}",
                            config: client_config)
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=auth_var").wait
        assert_instance_of Scada::DataValue, dv
        assert_equal 7, dv.value
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end

  it "works without encryption (no config)" do
    with_server do |server, port|
      server.add_variable("ns=1;s=plain_var", type: :string, value: "hello",
                          display_name: "Plain")

      Sync do |task|
        server_task = task.async { server.run }
        client = new_client("opc.tcp://localhost:#{port}")
        client.connect
        client_task = task.async { client.run }

        dv = client.read("ns=1;s=plain_var").wait
        assert_instance_of Scada::DataValue, dv
        assert_equal "hello", dv.value
      ensure
        client_task&.stop
        server_task&.stop
      end
    end
  end
end
