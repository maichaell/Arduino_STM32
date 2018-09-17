#include <HardwareCAN.h>
#include "changes.h"
#include "utility/can.h"

#define MOTOR_CONTROL_ID 0x667

CanMsg msgMotorControl;

// Instanciation of CAN interface
HardwareCAN canBus(CAN1_BASE);

int counter;
bool mode;
uint16 rx_isr;

void CANSetup(void)
{
  CAN_STATUS Stat;

  msgMotorControl.IDE = CAN_ID_STD;
  msgMotorControl.RTR = CAN_RTR_DATA;
  msgMotorControl.ID = MOTOR_CONTROL_ID;
  msgMotorControl.DLC = 2;
  msgMotorControl.Data[0] = 0x0;
  msgMotorControl.Data[1] = 0x0;

  canBus.map(CAN_GPIO_PD0_PD1);
  Stat = canBus.begin(CAN_SPEED_500, CAN_MODE_NORMAL);

  if (Stat != CAN_OK)
  {
    Serial.println("setup NOK");
  }

  canBus.filter(0, 0, 0);
  canBus.set_irq_mode(); // Use irq mode (recommended), so the handling of incoming messages
                         // will be performed at ease in a task or in the loop. The software fifo is 16 cells long,
                         // allowing at least 15 ms before processing the fifo is needed at 125 kbps
                         // when using polling mode, call CAN_RX0_IRQ_Handler periodically to save frames into software fifo
}

// Send one frame. Parameter is a pointer to a frame structure (above), that has previously been updated with data.
// If no mailbox is available, wait until one becomes empty. There are 3 mailboxes.
CAN_TX_MBX CANsend(CanMsg *pmsg)
{
  CAN_TX_MBX mbx;

  
    mbx = canBus.send(pmsg);
#ifdef USE_MULTITASK
    vTaskDelay(1); // Infinite loops are not multitasking-friendly
#endif
   //while (mbx == CAN_TX_NO_MBX); // Waiting outbound frames will eventually be sent, unless there is a CAN bus failure.
  return mbx;
}

// Process incoming messages
// Note : frames are not fully checked for correctness: DLC value is not checked, neither are the IDE and RTR fields. However, the data is guaranteed to be corrrect.
void ProcessMessages(void)
{
  CanMsg *r_msg;

  // Loop for every message in the fifo
  while ((r_msg = canBus.recv()) != NULL)
  {
    char buf[5];
    sprintf(buf, "%010u 0x%03x", micros(), r_msg->ID);
    Serial.print(buf);
    Serial.print(" ");
    for (int i = 0; i < r_msg->DLC; i++)
    {
      Serial.print(r_msg->Data[i], HEX);      
      Serial.print(" ");
    }
    Serial.println("");
    canBus.free(); // Remove processed message from buffer, whatever the identifier
  }

#ifdef USE_MULTITASK
  vTaskDelay(1); // Infinite loops are not multitasking-friendly
#endif
}

// The application program starts here
void setup()
{
  // put your setup code here, to run once:
  Serial.begin();
  CANSetup(); // Initialize the CAN module and prepare the message structures.
  pinMode(PB9, OUTPUT);
  }


void loop()
{
  // Process incoming messages periodically (should be often enough to avoid overflowing the fifo)
  ProcessMessages(); // Process all incoming messages, update local variables accordingly

  //toggle LED when message is received
  if (rx_isr != can_rx_isr_count()) {
    digitalWrite(PB9, !digitalRead(PB9));
  }

  rx_isr= can_rx_isr_count();
  if (millis() % 500 == 0)
  {
    CAN_TX_MBX ret = CANsend(&msgMotorControl);
    Serial.print("TSR:");
    CAN_Port* p = CAN1_BASE;
    Serial.println(CAN1_BASE->TSR, HEX);

    Serial.print("MSR:");    
    Serial.println(CAN1_BASE->TSR, HEX);
    
    msgMotorControl.Data[0] = ++counter;
    CanErrStatus status;
    canBus.error_status(CAN1_BASE, &status);
    char s[128];
    sprintf(s, "REC:%d, TEC:%d, LEC:%d, BUSOFF:%d, ERR_PASSIVE:%d, ERR_WARNING:%d",
            status.REC, status.TEC, status.LEC, status.BUSOFF, status.ERR_PASSIVE, status.ERR_WARNING);
    Serial.println(s);
    Serial.print("RX isr count:");
    Serial.println(can_rx_isr_count());
    
    
  }


  delay(1); // The delay must not be greater than the time to overflow the incoming fifo (here about 15 ms)
}
