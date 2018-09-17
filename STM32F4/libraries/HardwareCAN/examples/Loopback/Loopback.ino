#include <HardwareCAN.h>

// Instanciation of CAN interface
HardwareCAN canBus(CAN1_BASE);

int counter = 0;

void CANSetup(void)
{
  canBus.map(CAN_GPIO_PB8_PB9);

  //connect CAN transceiver to PB8/PB9 or pull PB8 HIGH for LOOPBACK to work.
  CAN_STATUS Stat = canBus.begin(CAN_SPEED_125, CAN_MODE_LOOPBACK); // Other speeds go from 125 kbps to 1000 kbps. CAN allows even more choices.

  if (Stat != CAN_OK)
  {
    Serial.println("setup NOK");
  }
  else
  {
    Serial.println("setup OK");
  }
  canBus.filter(0, 0, 0);
  canBus.set_irq_mode(); // Use irq mode (recommended), so the handling of incoming messages
                         // will be performed at ease in a task or in the loop. The software fifo is 16 cells long,
                         // allowing at least 15 ms before processing the fifo is needed at 125 kbps
}

// Send one frame. Parameter is a pointer to a frame structure (above), that has previously been updated with data.
// If no mailbox is available, wait until one becomes empty. There are 3 mailboxes.
CAN_TX_MBX CANsend(CanMsg *pmsg)
{
  CAN_TX_MBX mbx;

  do
  {
    mbx = canBus.send(pmsg);
#ifdef USE_MULTITASK
    vTaskDelay(1); // Infinite loops are not multitasking-friendly
#endif
  } while (mbx == CAN_TX_NO_MBX); // Waiting outbound frames will eventually be sent, unless there is a CAN bus failure.
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
}

void loop()
{
  ProcessMessages();

  CanMsg msg;
  msg.IDE = CAN_ID_STD;
  msg.RTR = CAN_RTR_DATA;
  msg.DLC = 2;
  msg.Data[0] = ++counter;
  msg.Data[1] = counter * 2;

  CANsend(&msg); // Send this frame

  delay(100); // The delay must not be greater than the time to overflow the incoming fifo (here about 15 ms)
}
