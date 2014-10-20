==========================
Control Node Code comments
==========================

.. contents:: Table of Contents

Reader could refer to the control node documentation for frame and configuration details.

Hardware
========
Control Node STM32 have some interfaces connected to other IC on the platform:
	USART1 connected to FTDI4232H and through it to A8
		used for communication between A8 and control node, command frame, response frame, polling frame
	SPI2 connected to RF231
		used for all control node radio functionalities
	I2C1 connected to INA226
		used for all control node power consumption functionalities
	I2C2 connected to open node
		used for communication between control node and open node, especially sensor simulation on control node

Main software libraries
=======================
The control node application relies on several libraries:
	freeRTOS is used as real time OS
	openlab, event library is used to have an event mechanism based on queue provided by freeRTOS
	openlab, soft_timer library is used to have an unsigned 32bits timestamp based on 32kHz clock

Control Node Input Output
=========================
Command frame are received from the serial line on UART so through UART(=USART1) IT.
	UART IT handler : vCharHandler_irq
		check if this is a command frame
			yes put an event in the EVENT_QUEUE_NETWORK, handler vRX_manager
				=> the handler manager to decode the command execute it and send a response

If the command is CONFIG_RADIOPOLL or CONFIG_POWER_POLL then in case of the command execution success, in addition to the response frame, specific measure frame will be sent periodically.

RESET_TIME
==========
New time is set at the command reception.
There is a time transition flag put in the event application queue.
Its main purpose is to sent an acknowledge frame to the gateway about time transition from previous to new time reference.
This ensure that at that transition event, changing the time has no impact on previous measure. All measure are managed by an event put in this FIFO queue.

However some interrupts with time stamping could occurs at the wrong moment, precisely during the handler of the time transition and before it has set the new time.
To prevent that issue, the time stamp of a measure is in relation with the current time of freeRTOS. In the handler of this measure, this time is converted to the time reference in 32kHz set by the RESET_TIME command. So the gateway will receive time in 32kHz.
Drawback is an additional jitter on time precision, freeRTOS is set at 1ms and there is a conversion of this time in 32kHz

When receiving a RESET_TIME command, the transition from previous configuration to the next one is managed according to the following.
Save new time
Put in the event queue named application queue, the time transition event.
At the execution of the time transition event handler, the time is effectively set. 

Power Frame
===========
The mainTask decode the received command and in case of CONFIG_POWER_POLL call 
	set_power_poll(xRxFrame.payload[0], xRxFrame.payload[1])
	set_power_poll(uint8_t config1, uint8_t config2)

The openlab event and fiteco_lib_gwt libraries are used.

event provide 2 queues, here the EVENT_QUEUE_APPLI is used.

In fiteco_lib_gwt, there is the ina226 interrupt routine
	current_sample_ready_isr(handler_arg_t arg)
which send an event in the queue.

On-going measurement description
--------------------------------
The ina226 signal by this interrupt that new measurements according to period and average configuration are available.
The handler for this queue event, calls the lib user handler with Power, Voltage, Current measurement and the current time as parameters.

This user lib handler pw_measure_handler(handler_arg_t measure_time, float v, float c, float p) aggregate in the pollingFrame the measure requested according to xRxFrame.payload[0] in PVC order. Then depending of the specification, the frame could be sent with 1 bunch of measurement only or then when max number of bunch is reached. Currently, if the period of measurement is above approximatively 50ms then each measure is sent in a dedicated frame, below they are aggregate. Approximatively means it depends if there is something in the event queue before this event to be treated.


Setup measurement description
-----------------------------
When receiving a CONFIG_POWER_POLL command, the transition from previous configuration to the next one is manage according to the following.
	ina226 measurement is stopped with help of fiteco_lib_gwt library.
	A specific event, which is a transition boundary event, is posted in the queue with a specific handler power_poll_transition.
	All the remaining event in the queue, especially previous power frame if there was measurement on-going, are processed before the call of power_poll_transition.
	This handler then update the configuration byte of the power frame with the new configuration, do the configuration.
New measurements are then started.

This guarantee a clean separation between old measurements and new measurements and also the integrity of the global configuration variable, xPowerFrame.conf.

Radio Frame
===========
To be meanigful, CONFIG_RADIO must be done before. However there is a default configuration done by openlab platform at the initialization.

radio_poll() is called and first check if it is to start or stop the periodic measurement.
A specification is to aggregate in one radio frame all measurement which could occurs before 50ms. So for periodic radio measure < 25ms there could be several measure in one frame. Above this value there is only one value in the radio frame.
The measure will be started at the time of command + 1 period.
The maximum period of measurement is currently limited by the phy layer at ~ 1s. A signed 16bits value express a delta time so a maximum value of 32767 tick for a 32768Hz frequency tick.

Here the latency is far from optimum. The phy layer post in the 


