.. _bme68x_iaq:

BME68X IAQ driver
#############

.. contents::
   :local:
   :depth: 2

You can use the BME68X IAQ driver to run the Bosch Sensor Environmental Cluster (BSEC) library in order to get Indoor Air Quality (IAQ) readings.

.. note::
   Using the BSEC library requires accepting a separate license agreement.
   For details, see `BSEC`_.

After you have read and accepted the terms of BSEC, you can enable and download it using following commands:

..code-block::
   west config manifest.group-filter +bsec
   west update

See `BME680`_ for more information about BME680.

The :ref:`bme68x` sample demonstrates how to run this driver in an application.

Configuration
#############

1. The :ref:`bme680` Zephyr driver has to be disabled, set the :kconfig:option:`CONFIG_BME680` Kconfig option to false.
#. Configure :kconfig:option:`CONFIG_SETTINGS` and a settings backend to save the persistent state of the BSEC library.
#. To enable this driver, use the :ref:`CONFIG_BME68X_IAQ <CONFIG_BME68X_IAQ>` Kconfig option.

API documentation
*****************

| Header file: :file:`drivers/sensor/bme68x/bme68x_iaq.h`
| Source file: :file:`drivers/sensor/bme68x/bme68x_iaq.c`
