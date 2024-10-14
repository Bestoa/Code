package projectname

import spinal.core._
import spinal.lib._
import spinal.lib.com.uart._

case class UartCtrlUsageExample() extends Component {
  val io = new Bundle {
    val uart = master(Uart())
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

  val writeValid = Reg(Bool()) init False

  uartCtrl.io.write.payload := uartCtrl.io.read.payload

  when (uartCtrl.io.read.valid) {
    writeValid := True
  }
  when (uartCtrl.io.write.ready) {
    writeValid := False
  }

  uartCtrl.io.write.valid := writeValid

}

object MyTopLevelVerilog extends App {
  Config.spinal.generateVerilog(UartCtrlUsageExample())
}

