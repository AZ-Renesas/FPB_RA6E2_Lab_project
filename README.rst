Overview
********

This application targets the Renesas FPB-RA6E2 evaluation board
(``fpb_ra6e2``), using the ``r7fa6e2bb3cfm`` SoC target. It blinks LED1
using the AGT counter and, when push button S1 is pressed, samples ADC
channel 0 and transmits the raw and millivolt values through UART0.

Building and Running
********************

Run these commands from the root of the west workspace:

.. code-block:: console

    west build -b fpb_ra6e2 applications/FPB_RA6E2_Lab_project
    west flash

The FPB-RA6E2 board definition and application overlay provide the LED,
button, AGT counter, ADC, and UART devicetree nodes used by the application.

Sample Output
=============

.. code-block:: console

    Raw value: 2048 | Voltage: 1650 mV
