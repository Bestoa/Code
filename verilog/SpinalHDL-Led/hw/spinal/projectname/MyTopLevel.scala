package projectname

import spinal.core._

// Hardware definition
case class Led() extends Component {
  val io = new Bundle {
    val led = out Bits(4 bits)
  }

  val counter = Reg(UInt(32 bits)) init 0
  counter := counter + 1;

  val led = Reg(Bits(4 bits)) init 1

  when (counter === 24_000_000) {
    counter := 0
    led := led(2 downto 0) ## led(3)
  }

  io.led := ~led
}

object MyTopLevelVerilog extends App {
  Config.spinal.generateVerilog(Led())
}

