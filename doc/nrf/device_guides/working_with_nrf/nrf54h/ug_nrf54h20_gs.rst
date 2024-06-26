:orphan:

.. _ug_nrf54h20_gs:

Getting started with the nRF54H20 PDK
#####################################

.. contents::
   :local:
   :depth: 2

This section gets you started with your nRF54H20 Preliminary Development Kit (PDK) using the |NCS|.
It tells you how to install the :ref:`multicore_hello_world` sample and perform a quick test of your DK.

If you have already set up your nRF54H20 PDK and want to learn more, see the following documentation:

* :ref:`installation` and :ref:`configuration_and_build` documentation to install the |NCS| and learn more about its development environment.
* ``ug_nrf54`` documentation for more advanced topics related to the nRF54H20.

If you want to go through an online training course to familiarize yourself with Bluetooth Low Energy and the development of Bluetooth LE applications, enroll in the `Bluetooth LE Fundamentals course`_ in the `Nordic Developer Academy`_.

.. _ug_nrf54h20_gs_requirements:

Minimum requirements
********************

Make sure you have all the required hardware and that your computer has one of the supported operating systems.

Hardware
========

* nRF54H20 PDK
* USB-C cable

Software
========

On your computer, one of the following operating systems:

* Microsoft Windows
* Ubuntu Linux
* macOS

|supported OS|

You also need to install `Git`_ or `Git for Windows`_ (on Linux and Mac, or Windows, respectively).

.. _ug_nrf54h20_gs_test_blinky:

Testing with the Blinky sample
******************************

The nRF54H20 PDK comes preprogrammed with the :zephyr:code-sample:`blinky` sample.

Complete the following steps to test that the PDK works correctly:

1. Connect the USB-C end of the USB-C cable to the **IMCU USB** port the nRF54H20 PDK.
#. Connect the other end of the USB-C cable to your PC.
#. Move the **POWER** switch to **On** to turn the nRF54H20 PDK on.
   The switch is located in the top left corner of the PDK PCB.

**LED1** will turn on and start to blink.

If something does not work as expected, contact Nordic Semiconductor support.

.. _nrf54h20_gs_installing_software:

Installing the required software
********************************

On your computer, install `nRF Connect for Desktop`_.
You must also install a terminal emulator, such as `nRF Connect Serial Terminal`_ (from the nRF Connect for Desktop application) or the nRF Terminal (part of the `nRF Connect for Visual Studio Code`_ extension).

.. _ug_nrf54h20_gs_sample:

Programming the sample
**********************

The :ref:`multicore_hello_world` sample is a multicore sample running on both the Application core (``cpuapp``) and the Peripheral Processor (PPR, ``cpuppr``).
It uses the ``nrf54h20dk_nrf54h20_cpuapp@soc1`` build target.

To build and program the sample to the nRF54H20 PDK, complete the following steps:

1. Connect the nRF54H20 PDK to you computer using the IMCU USB port on the PDK.
#. Navigate to the :file:`nrf/samples/multicore/hello_world` folder containing the sample.
#. Build the sample by running the following command::

      west build -b nrf54h20dk_nrf54h20_cpuapp@soc1

#. Program the sample using nrfjprog.
   If you have multiple Nordic Semiconductor devices, make sure that only the nRF54H20 PDK you want to program is connected.

   .. code-block:: console

      west flash

The sample will be automatically built and programmed on both the Application core and the Peripheral Processor (PPR) of the nRF54H20.

.. _nrf54h20_sample_reading_logs:

Reading the logs
****************

With the :ref:`multicore_hello_world` sample programmed, the nRF54H20 PDK outputs logs for the application core and the peripheral processor.
The logs are output over UART.

To read the logs from the :ref:`multicore_hello_world` sample programmed to the nRF54H20 PDK, complete the following steps:

1. Connect to the PDK with a terminal emulator (for example, `Serial Terminal from nRF Connect for Desktop`_) using the following settings:

   * Baud rate: 115200
   * 8 data bits
   * 1 stop bit
   * No parity
   * HW flow control: None

#. Press the **Reset** button on the PCB to reset the PDK.
#. Observe the console output for both cores:

   * For the application core, the output should be as follows:

     .. code-block:: console

        *** Booting Zephyr OS build v2.7.99-ncs1-2193-gd359a86abf14  ***
        Hello world from nrf54h20dk_nrf54h20_cpuapp

   * For the PPR core, the output should be as follows:

     .. code-block:: console

        *** Booting Zephyr OS build v2.7.99-ncs1-2193-gd359a86abf14  ***
        Hello world from nrf54h20dk_nrf54h20_cpuppr

.. note::
   If no output is shown when using nRF Serial Terminal, select a different serial port in the terminal application.

See the :ref:`ug_nrf54h20_logging` page for more information.

Next steps
**********

You are now all set to use the nRF54H20 PDK.
See the following links for where to go next:

* :ref:`ug_nrf54h20_architecture` for information about the multicore System-on-Chip, such as the responsibilities of the cores and their interprocessor interactions, the memory mapping, and the boot sequence.
* The :ref:`introductory documentation <getting_started>` for more information on the |NCS| and the development environment.
