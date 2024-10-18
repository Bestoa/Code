package projectname

import spinal.core._
import spinal.lib._

// Hardware definition
case class MemDemo() extends Component {
  val io = new Bundle {
    val led = out Bits(4 bits)
  }

  def ledTable = for(i <- 0 until 16) yield {
    B(i % 16)

  }
  val ram =  Mem(Bits(4 bits), initialContent = ledTable)

  val counter = Counter(24_000_000)
  val addr = Reg(UInt(4 bits)) init 0
  val led = Reg(Bits(4 bits))

  val lfsr = new Area {
    val value = Reg(Bits(4 bits)) init 1
    val feedback = value(0)
    when (counter.willOverflow) {
      value(0) := value(1)
      value(1) := value(2)
      value(2) := value(3) ^ feedback
      value(3) := feedback
    }
  }

  counter.increment()
  when (counter.willOverflow) {
    addr := addr + 1
    when (addr === 15) {
      addr := 0
    }
    ram.write(addr, lfsr.value)
  }

  led := ram.readSync(addr)
  io.led := ~led
}

object MyTopLevelVerilog extends App {
  Config.spinal.generateVerilog(MemDemo())
}

