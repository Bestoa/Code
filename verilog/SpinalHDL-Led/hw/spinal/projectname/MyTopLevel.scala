package projectname

import spinal.core._
import spinal.lib._

case class PLL() extends BlackBox {
  val io = new Bundle {
    val clkin = in Bool()
    val clkout0 = out Bool()
    val locked = out Bool()
  }
  noIoPrefix()
}

// Hardware definition
case class Led() extends Component {
  val io = new Bundle {
    val led = out Bits(4 bits)
  }

  // Create an Area to manage all clocks and reset things
  val clkCtrl = new Area {
    // Instantiate and drive the PLL
    val pll = new PLL
    pll.io.clkin := ClockDomain.current.readClockWire

    // Create a new clock domain named 'core'
    val coreClockDomain = ClockDomain.internal(
      name = "core",
      frequency = FixedFrequency(48 MHz)  // This frequency specification can be used
    )                                      // by coreClockDomain users to do some calculations

    // Drive clock and reset signals of the coreClockDomain previously created
    coreClockDomain.clock := pll.io.clkout0
    coreClockDomain.reset := ClockDomain.current.readResetWire && pll.io.locked
  }

  val core = new ClockingArea(clkCtrl.coreClockDomain) {
    val counter = Counter(24_000_000)
    val led = Reg(Bits(4 bits)) init 1

    counter.increment()
    when (counter.willOverflow) {
      counter.clear()
      led := led(2 downto 0) ## led(3)
    }
    io.led := ~led
  }

}

object MyTopLevelVerilog extends App {
  Config.spinal.generateVerilog(Led())
}

