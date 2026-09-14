Overview
********

This application targets the Renesas RA6E2 custom board
(``ra6e2_custom``). It blinks LED1 using the AGT counter and, when push
button S1 is pressed, samples ADC channel 0 and transmits the raw and
millivolt values through UART0.

Building and Running
********************

Run these commands from the root of the west workspace:

.. code-block:: console

    west build -b ra6e2_custom applications/FPB_RA6E2_Lab_project
    west flash

The board definition and application overlay provide the LED, button, AGT
counter, ADC, and UART devicetree nodes used by the application.

Sample Output
=============

.. code-block:: console

    Raw value: 2048 | Voltage: 1650 mV
