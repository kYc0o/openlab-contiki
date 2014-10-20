Test stack usage sscanf
=======================

`sscanf` requires at least 212 of stack without parameter so the stack should be
sized accordingly.
`configMINIMAL_STACK_SIZE` is not enough so scanf cannot be called from event
handler context.


Usage
-----

Platform `FreeRTOSConfig.h` should be modified to add stack monitoring support

    #define INCLUDE_uxTaskGetStackHighWaterMark 1

Output
------

    Base stack 390
    Post empty scanf   178 diff 212
    Post simple scanf  163 diff 227
    Post complex scanf 163 diff 227

    Free stack event queue 168



