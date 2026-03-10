module Scada
  StatusCode = Data.define(:code) do
    def good? = (code & 0xC0000000) == 0
    def bad?  = (code & 0x80000000) != 0
    def uncertain? = !good? && !bad?

    def to_s
      "0x#{code.to_s(16).rjust(8, '0')}"
    end

    def message
      to_s
    end

    def inspect
      "#<Scada::StatusCode #{to_s}>"
    end
  end
end
