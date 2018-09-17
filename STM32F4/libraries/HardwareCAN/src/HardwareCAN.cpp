/**
 * @brief HardwareCAN "wiring-like" api for CAN
 */

#include "wirish.h"
#include "utility/can.h"
#include "HardwareCAN.h"

/**
 * @brief Initialize a CAN peripheral
 * @param freq frequency to run at, must one of the following values:
 *           - CAN_SPEED_1000
 *           - CAN_SPEED_500
 *           - CAN_SPEED_250
 *           - CAN_SPEED_125
 */
CAN_STATUS HardwareCAN::begin(CAN_SPEED speed, uint32 mode)
{
	int8_t init=can_init(Port, CAN_MCR_ABOM| CAN_MCR_TTCM, speed);
	
	if (init != CAN_OK) {
		//print ESR value
	}

	return can_set_mode(Port, mode);
}

void HardwareCAN::set_pool_mode(void)
{
	return can_set_pool_mode(Port);
}

void HardwareCAN::set_irq_mode(void)
{
	return can_set_irq_mode(Port);
}

CAN_STATUS HardwareCAN::filter(uint8 idx, uint32 id, uint32 mask)
{
	return can_filter(Port, idx, CAN_FIFO0, CAN_FILTER_32BIT, CAN_FILTER_MASK, id, mask);
}

CAN_STATUS HardwareCAN::status(void)
{
	return can_status();
}

CAN_TX_MBX HardwareCAN::send(CanMsg* message)
{
	return can_transmit(Port, message);
}

uint8 HardwareCAN::available(void)
{
	return can_rx_available();
}

uint8 HardwareCAN::frame_lost(void)
{
	return can_frame_lost() ;
}

CanMsg* HardwareCAN::recv(void)
{
	return can_rx_queue_get();
}

void HardwareCAN::clear(void)
{
	can_rx_queue_clear();
}

void HardwareCAN::free(void)
{
	can_rx_queue_free();
}

void HardwareCAN::cancel(CAN_TX_MBX mbx)
{
	can_cancel(Port, mbx);
}

/**
 * @brief Initialize a CAN peripheral
 */
CAN_STATUS HardwareCAN::begin(void)
{
	return begin(CAN_SPEED_250, CAN_MODE_NORMAL);
}

void HardwareCAN::end(void)
{
	can_deinit(Port);
}

CanMsg* HardwareCAN::read(CAN_FIFO fifo, CanMsg* msg)
{
	return can_read(Port, fifo, msg);
}

void HardwareCAN::release(CAN_FIFO fifo)
{
	can_rx_release(Port, fifo);
}

/**
 * @brief Initialize a CAN peripheral
 */
CAN_STATUS HardwareCAN::map(CAN_GPIO_MAP remap)
{
	return can_gpio_map(Port, remap);
}

uint8 HardwareCAN::fifo_ready(CAN_FIFO fifo)
{
	return can_fifo_ready(Port, fifo);
}

void HardwareCAN::error_status(CAN_Port *CANx, CanErrStatus* status) {
	can_err_status(CANx, status);
}

HardwareCAN::HardwareCAN(CAN_Port* CANx)
{
	Port = CANx;
}

