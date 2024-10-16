package projectname

import spinal.core._
import spinal.lib._
import spinal.lib.com.uart._

case class UartCtrlUsageExample() extends Component {
  val io = new Bundle {
    val uart = master(Uart())
    var led = out Bits(4 bits)
  }

  val uartCtrl = new UartCtrl()
  // set config manually to show that this is still OK
  uartCtrl.io.config.setClockDivider(115200 Hz, 24 MHz)
  uartCtrl.io.config.frame.dataLength := 7  // 8 bits
  uartCtrl.io.config.frame.parity := UartParityType.NONE
  uartCtrl.io.config.frame.stop := UartStopType.ONE
  uartCtrl.io.uart <> io.uart

  uartCtrl.io.writeBreak := False
  uartCtrl.io.read.ready := True

  val writeValid = Reg(Bool()) init True
  val cnt = Reg(UInt(32 bits)) init 0
  val enableEcho = Reg(Bool()) init False
  val index = Reg(UInt(32 bits)) init 0
  val hello_str = "Welcome to SpinalHDL's world-1!\n\r"
  
  val hello = Bits(hello_str.length*8 bits)
  for (i <- 0 until hello_str.length) {
    hello((i+1)*8-1 downto i*8) := hello_str(i)
  }


  cnt := cnt + 1
  when (cnt === 24_000_000) {
    writeValid := True
    cnt := 0
    index := 0
    enableEcho := False
  }

  when (enableEcho) {
    uartCtrl.io.write.payload := uartCtrl.io.read.payload
    writeValid.setWhen(uartCtrl.io.read.valid)
  } otherwise {
    uartCtrl.io.write.payload := hello((index)*8, 8 bits)
  }

  when (uartCtrl.io.write.ready) {
    when (index < hello_str.length - 1) {
      index := index + 1
    } otherwise {
      enableEcho := True
      writeValid := False
    }
  }

  uartCtrl.io.write.valid := writeValid

  io.led := ~uartCtrl.io.read.payload(0, 4 bits)
}

object MyTopLevelVerilog extends App {
  Config.spinal.generateVerilog(UartCtrlUsageExample())
}

