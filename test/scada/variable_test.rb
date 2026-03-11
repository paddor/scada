require_relative "../test_helper"

describe "Variables" do
  include ScadaHelper

  @fixture = ScadaHelper.start_shared_server { |s|
    s.add_variable('ns=1;s=temp', type: :double, value: 22.0,
                    display_name: 'Temperature')
    s.add_variable('ns=1;s=setpoint', type: :double, value: 20.0,
                    display_name: 'Setpoint')
    s.add_variable('ns=1;s=name', type: :string, value: 'initial',
                    display_name: 'Name')
    s.add_variable('ns=1;s=count', type: :int32, value: 0,
                    display_name: 'Count')
    s.add_variable('ns=1;s=flag', type: :boolean, value: false,
                    display_name: 'Flag')
    s.add_variable('ns=1;s=sbyte', type: :sbyte, value: -42,
                    display_name: 'SByte')
    s.add_variable('ns=1;s=byte', type: :byte, value: 200,
                    display_name: 'Byte')
    s.add_variable('ns=1;s=i16', type: :int16, value: -1000,
                    display_name: 'Int16')
    s.add_variable('ns=1;s=u16', type: :uint16, value: 65535,
                    display_name: 'UInt16')
    s.add_variable('ns=1;s=u32', type: :uint32, value: 3_000_000_000,
                    display_name: 'UInt32')
    s.add_variable('ns=1;s=i64', type: :int64, value: -9_000_000_000,
                    display_name: 'Int64')
    s.add_variable('ns=1;s=u64', type: :uint64,
                    value: 18_000_000_000_000_000_000,
                    display_name: 'UInt64')
    s.add_variable('ns=1;s=dbl', type: :double,
                    value: 1.23456789012345e+200,
                    display_name: 'Double')
    s.add_variable('ns=1;s=flt', type: :float, value: 3.14,
                    display_name: 'Float')
    s.add_variable('ns=1;s=ts', type: :datetime,
                    value: Time.utc(2025, 6, 15, 12, 30, 45),
                    display_name: 'Timestamp')
    s.add_variable('ns=1;s=blob', type: :byte_string,
                    value: "\x00\x01\xFF\xFE".b,
                    display_name: 'Blob')
    s.add_variable('ns=1;s=temps', type: :double,
                    value: [20.0, 21.5, 22.0],
                    display_name: 'Temperatures')
    s.add_variable('ns=1;s=ids', type: :int32, value: [1, 2, 3],
                    display_name: 'IDs')
    s.add_variable('ns=1;s=tags', type: :string, value: ['a', 'b'],
                    display_name: 'Tags')
    s.add_variable('ns=1;s=bits', type: :boolean,
                    value: [true, false, true],
                    display_name: 'Bits')
  }

  it "reads initial double value" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=temp').wait
      assert_instance_of Scada::DataValue, dv
      assert_in_delta 22.0, dv.value, 0.01
      assert dv.status.good?
    end
  end

  it "writes and reads back a double" do
    with_client(fixture) do |client|
      client.write('ns=1;s=setpoint', 35.0).wait
      dv = client.read('ns=1;s=setpoint').wait
      assert_in_delta 35.0, dv.value, 0.01
    end
  end

  it "writes and reads a string variable" do
    with_client(fixture) do |client|
      client.write('ns=1;s=name', 'updated').wait
      dv = client.read('ns=1;s=name').wait
      assert_equal 'updated', dv.value
    end
  end

  it "writes and reads an integer variable" do
    with_client(fixture) do |client|
      client.write('ns=1;s=count', 42).wait
      dv = client.read('ns=1;s=count').wait
      assert_equal 42, dv.value
    end
  end

  it "writes and reads a boolean variable" do
    with_client(fixture) do |client|
      client.write('ns=1;s=flag', true).wait
      dv = client.read('ns=1;s=flag').wait
      assert_equal true, dv.value
    end
  end

  # --- Small integer types ---

  it "round-trips a SByte (int8)" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=sbyte').wait
      assert_equal(-42, dv.value)

      client.write('ns=1;s=sbyte', 127, type: :sbyte).wait
      dv = client.read('ns=1;s=sbyte').wait
      assert_equal 127, dv.value
    end
  end

  it "round-trips a Byte (uint8)" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=byte').wait
      assert_equal 200, dv.value

      client.write('ns=1;s=byte', 0, type: :byte).wait
      dv = client.read('ns=1;s=byte').wait
      assert_equal 0, dv.value
    end
  end

  it "round-trips an Int16" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=i16').wait
      assert_equal(-1000, dv.value)

      client.write('ns=1;s=i16', 32767, type: :int16).wait
      dv = client.read('ns=1;s=i16').wait
      assert_equal 32767, dv.value
    end
  end

  it "round-trips a UInt16" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=u16').wait
      assert_equal 65535, dv.value

      client.write('ns=1;s=u16', 0, type: :uint16).wait
      dv = client.read('ns=1;s=u16').wait
      assert_equal 0, dv.value
    end
  end

  # --- Larger integer types ---

  it "round-trips a UInt32" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=u32').wait
      assert_equal 3_000_000_000, dv.value

      client.write('ns=1;s=u32', 0, type: :uint32).wait
      dv = client.read('ns=1;s=u32').wait
      assert_equal 0, dv.value
    end
  end

  it "round-trips an Int64" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=i64').wait
      assert_equal(-9_000_000_000, dv.value)

      client.write('ns=1;s=i64', 9_000_000_000, type: :int64).wait
      dv = client.read('ns=1;s=i64').wait
      assert_equal 9_000_000_000, dv.value
    end
  end

  it "round-trips a UInt64" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=u64').wait
      assert_equal 18_000_000_000_000_000_000, dv.value

      client.write('ns=1;s=u64', 42, type: :uint64).wait
      dv = client.read('ns=1;s=u64').wait
      assert_equal 42, dv.value
    end
  end

  # --- Double (64-bit float) ---

  it "round-trips a Double (64-bit float)" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=dbl').wait
      assert_in_delta 1.23456789012345e+200, dv.value, 1e+187

      client.write('ns=1;s=dbl', -1.7976931348623157e+308).wait
      dv = client.read('ns=1;s=dbl').wait
      assert_in_delta(-1.7976931348623157e+308, dv.value, 1e+294)
    end
  end

  # --- Float ---

  it "round-trips a Float (32-bit)" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=flt').wait
      assert_in_delta 3.14, dv.value, 0.001

      client.write('ns=1;s=flt', -273.15, type: :float).wait
      dv = client.read('ns=1;s=flt').wait
      assert_in_delta(-273.15, dv.value, 0.1)
    end
  end

  # --- DateTime ---

  it "round-trips a DateTime" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=ts').wait
      assert_instance_of Time, dv.value
      assert_equal 2025, dv.value.year
      assert_equal 6, dv.value.month
      assert_equal 15, dv.value.day
      assert_equal 12, dv.value.hour
    end
  end

  # --- ByteString ---

  it "round-trips a ByteString" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=blob').wait
      assert_equal Encoding::ASCII_8BIT, dv.value.encoding
      assert_equal "\x00\x01\xFF\xFE".b, dv.value
    end
  end

  # --- Arrays ---

  it "reads an array of doubles" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=temps').wait
      assert_instance_of Array, dv.value
      assert_equal 3, dv.value.length
      assert_in_delta 20.0, dv.value[0], 0.01
      assert_in_delta 21.5, dv.value[1], 0.01
      assert_in_delta 22.0, dv.value[2], 0.01
    end
  end

  it "round-trips an array of int32" do
    with_client(fixture) do |client|
      client.write('ns=1;s=ids', [10, 20, 30], type: :int32).wait
      dv = client.read('ns=1;s=ids').wait
      assert_equal [10, 20, 30], dv.value
    end
  end

  it "round-trips an array of strings" do
    with_client(fixture) do |client|
      client.write('ns=1;s=tags', ['x', 'y', 'z'], type: :string).wait
      dv = client.read('ns=1;s=tags').wait
      assert_equal ['x', 'y', 'z'], dv.value
    end
  end

  it "round-trips an array of booleans" do
    with_client(fixture) do |client|
      dv = client.read('ns=1;s=bits').wait
      assert_equal [true, false, true], dv.value

      client.write('ns=1;s=bits', [false, true], type: :boolean).wait
      dv = client.read('ns=1;s=bits').wait
      assert_equal [false, true], dv.value
    end
  end
end
