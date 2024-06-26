.. _ug_wifi_mem_req_scan_mode:

Memory requirements for Wi-Fi applications in Scan mode
#######################################################

Code and RAM memory footprint requirements differ depending on the selected platform and the application example.

The following tables list memory requirement values for selected Wi-Fi® samples supporting Scan mode.

Footprint values are provided in kilobytes (KB).

.. tabs::

   .. tab:: nRF52840 DK

      The following table lists memory requirements for samples running on the :ref:`nRF52840 DK <programming_board_names>` (:ref:`nrf52840dk_nrf52840 <zephyr:nrf52840dk_nrf52840>`).

      +--------------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | Sample                               |   Total ROM |   Wi-Fi driver ROM (incl. nRF70 FW patch) |   WPA supplicant ROM |   Total RAM (incl. static HEAP) |   Wi-Fi driver RAM |   WPA supplicant RAM |
      +======================================+=============+===========================================+======================+=================================+====================+======================+
      | :ref:`Scan <wifi_scan_sample>`       |         106 |                                        36 |                    0 |                              76 |                 44 |                    0 |
      +--------------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+

   .. tab:: nRF7002 DK

      The following table lists memory requirements for samples running on the :ref:`nRF7002 DK <programming_board_names>` (:ref:`nrf7002dk_nrf5340_cpuapp <nrf7002dk_nrf5340>`).

      +--------------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | Sample                               |   Total ROM |   Wi-Fi driver ROM (incl. nRF70 FW patch) |   WPA supplicant ROM |   Total RAM (incl. static HEAP) |   Wi-Fi driver RAM |   WPA supplicant RAM |
      +======================================+=============+===========================================+======================+=================================+====================+======================+
      | :ref:`Scan <wifi_scan_sample>`       |         104 |                                        38 |                    0 |                              74 |                 44 |                    0 |
      +--------------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+

   .. tab:: nRF9160 DK

      The following table lists memory requirements for samples running on the :ref:`nRF9160 DK <programming_board_names>` (:ref:`nrf9160dk_nrf9160_ns <zephyr:nrf9160dk_nrf9160>`).

      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | Sample                            |   Total ROM |   Wi-Fi driver ROM (incl. nRF70 FW patch) |   WPA supplicant ROM |   Total RAM (incl. static HEAP) |   Wi-Fi driver RAM |   WPA supplicant RAM |
      +===================================+=============+===========================================+======================+=================================+====================+======================+
      | :ref:`Location <location_sample>` |         213 |                                        39 |                    0 |                             103 |                 69 |                    0 |
      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | :ref:`Scan <wifi_scan_sample>`    |          96 |                                        36 |                    0 |                              71 |                 64 |                    0 |
      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+

   .. tab:: nRF9161 DK

      The following table lists memory requirements for samples running on the :ref:`nRF9161 DK <programming_board_names>` (:ref:`nrf9161dk_nrf9161_ns <zephyr:nrf9161dk_nrf9161>`).

      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | Sample                            |   Total ROM |   Wi-Fi driver ROM (incl. nRF70 FW patch) |   WPA supplicant ROM |   Total RAM (incl. static HEAP) |   Wi-Fi driver RAM |   WPA supplicant RAM |
      +===================================+=============+===========================================+======================+=================================+====================+======================+
      | :ref:`Location <location_sample>` |         236 |                                        11 |                    0 |                             103 |                 50 |                    0 |
      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
      | :ref:`Scan <wifi_scan_sample>`    |         116 |                                         9 |                    0 |                              71 |                 44 |                    0 |
      +-----------------------------------+-------------+-------------------------------------------+----------------------+---------------------------------+--------------------+----------------------+
