#!/usr/bin/ruby

%x[stty -F /dev/ttyUSB0 57600 -hup raw -echo]

require "serialport"
require "yaml"

#params for serial port
port_str = "/dev/ttyUSB0"  #may be different for you
baud_rate = 57600
data_bits = 8
stop_bits = 1
parity = SerialPort::NONE
DEBUG = false

prefix = "energy.onewire"

sp = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)

name_mapping = YAML.load(File.read("mapping.yaml"))

$stderr.puts "OneWire JeeLink initialized, now waiting for input..." if DEBUG
#just read forever
while true do
	while (i = sp.gets.gsub(/\0/, "").chomp) do
		data = i.split(" ")
		result = {
			status: data.shift,
			timestamp: data.shift,
			id: data.shift(8).map{|i| "%02x" % i.to_i},
			data: data.map(&:to_i),
		}

		case result[:id].join(":")
		when /^00:00:/
			data = Array.new(result[:data])
			type = data.shift

			result[:type] = case type
					when 1
						"heartbeat"
					else
						"unknown"
					end

			result[:value] = data.inject([0, 0]){|r, i|
				e, index = r
				e = e + i * 2**index
				index += 8
				[e, index]
			}.first
		when /^28:/
			low, high = result[:data].first(2)

			high = high << 8
			t = high + low

			sign = t & 32768

			if sign == 0
				s = 1
			else
				s = -1
				t = (t ^ 65535) + 1
			end
			t = (s * t / 16.0).round(1)

			result[:value] = t
			result[:type] = "temperature"
		end

		result[:name] = name_mapping[result[:id].join] || result[:id].join

		$stderr.puts result.inspect if DEBUG
		if result[:type] && result[:value]
			puts "#{prefix}.#{result[:name]}.#{result[:type]}.value #{result[:value]} #{Time.now.to_i}"
			$stdout.flush
		end
	end
end

sp.close
