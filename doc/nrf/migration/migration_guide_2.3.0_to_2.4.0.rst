.. _ncs_2.4.0_migration:

Migration notes for |NCS| v2.4.0
################################

.. contents::
   :local:
   :depth: 2

In version 2.4.0, the |NCS| introduced a number of major changes into its codebase.
If your application was built using |NCS| v2.3.0, make sure to complete the actions listed in the following sections to migrate your application to |NCS| v2.4.0.


Zephyr changes
**************

With Zephyr v3.x.x, TinyCBOR becomes deprecated and is replaced by zcbor.
If you are using TinyCBOR to encode data into CBOR format in your application, follow this guide to migrate your code.

.. note::
   If you have extended MCUmgr with additional custom commands, then your code will not compile anymore as the commands will receive pointers to variables that no longer exist or that have changed their types during TinyCBOR to zcbor transition.

.. _migration_tinycbor_to_zcbor_general:

General migration
=================

The TinyCBOR to zcbor migration consists of two tasks:

1. :ref:`migration_tinycbor_to_zcbor_general_init`
#. :ref:`migration_tinycbor_to_zcbor_general_api`

The first task involves context initialization, while the second task is almost a function-to-function conversion, with a few exceptions in how parameters are passed and what return values are returned from API calls.

.. _migration_tinycbor_to_zcbor_general_init:

zcbor state initialization
--------------------------

You must first initialize the decoding and encoding state of zcbor.

Decoding state initialization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Decoding with TinyCBOR requires setting up a reader with a buffer by calling ``cbor_buf_reader_init()``, and attaching it to a parser state with ``cbor_parser_init()``.
zcbor decoding is initialized with a call to ``zcbor_new_decode_state()`` that directly ties the provided buffer to decoding state.
For better understanding of how zcbor states work, refer to the `zcbor documentation`_.

The following is an example of simple zcbor state initialization for decoding:

.. code-block:: c

   #define N   20              /* Max expected encoded elements */
   zcbor_state_t zs[2];
   uint8_t buffer[SOME_SIZE];
   /* Read some data to the buffer */
   ...
   zcbor_new_decode_state(zs, ARRAY_SIZE(zs), buffer, ARRAY_SIZE(buffer), N);
   /* Decode data from the buffer with zs state */
   ...



MCUmgr commands and groups migration
====================================

In contrast to :ref:`migration_tinycbor_to_zcbor_general`, except for tuning some Kconfig configuration constants, zcbor setup is not needed for the MCUmgr commands and groups, because the MCUmgr library takes care of the task.
The only exception is tuning some Kconfig configuration constants.

Most of the function replacement task is covered by the :ref:`migration_tinycbor_to_zcbor_general`, with exception to usage of ``cborattr``, which is covered in :ref:`migration_tinycbor_to_zcbor_mcumgr_cborattr`.

The other important issue is decoding or encoding context access, which is described in :ref:`migration_tinycbor_to_zcbor_mcumgr_ctxt`.

.. note::

   The encoding context provided by the command processing handler function as a parameter has the top level map already created.
   The map will be closed on a successful return from the handler, so handlers need not create the top level map or close it.
   The decoding context, in contrary to the encoding, is set before the top map and handlers are responsible for opening of the top-level map as a part of command decoding or processing code.

.. _migration_tinycbor_to_zcbor_mcumgr_ctxt:

Decoding and encoding context
-----------------------------

Whenever TinyCBOR or zcbor is used, command processing functions are given a pointer to an object of type ``struct mgmt_ctxt``.
TinyCBOR uses the ``encoder`` element of this object for encoding functions and ``it``, a TinyCBOR ``CborValue`` type iterator, for decoding functions.

zcbor uses ``zcbor_state_t`` type objects for decoding and encoding states.
These state objects have indirectly replaced TinyCBOR's ``it`` iterator and ``encoder``, respectively, within ``struct mgmt_ctxt``.
They are embedded within the ``struct cbor_nb_reader`` type object for decoding context, and in the ``struct cbor_nb_writer`` type object for encoding context.

The ``struct cbor_nb_reader`` and ``struct cbor_nb_writer`` objects tie ``zcbor_state_t`` with ``net_buf`` type buffers that hold data for decoding or will hold encoded data.

The substitution, in code, of references to decoder and encoder objects, as accessed through ``struct mgmt_ctxt``, is shown by the following table:

+-------------------+-------------------+
| TinyCBOR          | zcbor             |
+===================+===================+
| ``encoder``       | ``cnbe.ts``       |
|                   +-------------------+
|                   | ``cnbe->ts``      |
+-------------------+-------------------+
| ``it``            | ``cnde.ts``       |
|                   +-------------------+
|                   | ``cnde->ts``      |
+-------------------+-------------------+

.. _migration_tinycbor_to_zcbor_mcumgr_cborattr:

Replacing ``cbor_read_object()`` with ``zcbor_map_decode_bulk()``
-----------------------------------------------------------------

.. note::

   In scenarios where decoding of keys is not required or it is required for only a single key, the procedure described in this section can be greatly simplified as a single key can be obtained by using ``zcbor_tst_decode()`` that is looped until the key is found.

``cborattr`` was a private MCUmgr utility that was used within the command processing code to process CBOR list contents in bulk and is now replaced with ``zcbor_bulk()``.
These utility APIs are represented by a single function: ``cbor_read_object()`` in case of TinyCBOR, and ``zcbor_map_decode_bulk()`` in case of zcbor.

The advantage of ``zcbor_map_decode_bulk()`` over ``cbor_read_object()`` is the simplification of structures that define the mapping of decoding functions to keys in a CBOR map.

The following example demonstrates the transition from ``cbor_read_object()`` to ``zcbor_map_decode_bulk()``:

.. code-block:: c

   int image;
   uint8_t img_data[SOME_DATA_LEN];
   size_t data_len;
   const struct cbor_attr_t off_attr[] = {
       [0] = {
           .attribute = "image",
           .type = CborAttrUnsignedIntegerType,
           .addr.uinteger = &req.image,
           .nodefault = true
       },
       [1] = {
           .attribute = "data",
           .type = CborAttrByteStringType,
           .addr.bytestring.data = img_data,
           .addr.bytestring.len = &data_len,
           .len = sizeof(img_data)
       },
       ...
   }
   rc = cbor_read_object(&ctxt->it, off_attr);
   ...

The ``off_attr`` provides specification of data encoded in CBOR format that is decoded by the call to ``cbor_read_object()``.
The specification is a list of structures that specify the following elements:

* An expected list key (``.attribute`` element)
* A type of key (``.type``)
* A pointer to the buffer for the key (usually subelement of the ``.addr`` element)
* A few other attributes

The ``cbor_read_object()`` function takes the specification and attempts to get all the described fields into the specified designated variables, using the TinyCBOR decoding context.
After a successful call to ``cbor_read_object()``, the variables are set and ready for further processing, and the buffer is shifted beyond the list.
This also includes copying binary and string buffers to the specified locations.

The context is accessed by ``ctxt->it`` in the given example.
It is provided by the MCUmgr library, so it is already initialized and has the buffer attached.

The following code sample is the zcbor conversion of the given code example:

.. code-block:: c

   int image;
   struct zcbor_string zst;
   struct zcbor_map_decode_key_val image_upload_decode[] = {
           ZCBOR_MAP_DECODE_KEY_VAL(image, zcbor_int32_decode, &image),
           ZCBOR_MAP_DECODE_KEY_VAL(data, zcbor_bstr_decode, &zst),
   };
   ok = zcbor_map_decode_bulk(zsd, image_upload_decode,
                              ARRAY_SIZE(image_upload_decode), &decoded) == 0;
   ...

The list specification is significantly simplified as it consists of an array of ``ZCBOR_MAP_DECODE_KEY_VAL`` defined mappings, each of which is provided with the following parameters:

* Name of a key (without quotation marks)
* zcbor decoding function directly from API (or user-provided type equivalent)
* Pointer to a variable where the extracted value will be stored

The difference here is that the decoding of binary and string data does not involve copying of the data to a specified buffer.
Instead, as described in :ref:`migration_tinycbor_to_zcbor_general`, the decoding function is paired with a ``struct zcbor_string`` type variable where decoding will store position and length of the data, within the provided CBOR buffer.
It is then up to the user to copy the values to a specified location.
This mechanism improves the RAM and CPU usage because the need for an intermediate buffer is often reduced.

To process a CBOR buffer and decode it according to the defined mapping, ``zcbor_map_decode_bulk()`` takes the following parameters:

* zcbor decoding context (``zsd`` in the above example)
* Mapping specification and size of the mapping
* Pointer to a variable for number of successfully decoded map elements

.. note::
   Currently, there is no method provided to determine which keys have not been found in a decoded map.
   It is up to the user to verify whether keys that are optional have appeared in the decoded string.
   This can be done, for example, by checking if a destination variable value has changed from the initial value.

New Zephyr logging service - logging v2
***************************************

Zephyr v3.x.x uses a new logging service (logging v2) by default.
The legacy version of logging is still supported but it is deprecated and will be removed after Zephyr v3.1.0.
For details about the Zephyr logging service, see :ref:`zephyr:logging_api`.
Version 2 supports the same set of features with a number of extensions, however, the logging backend API is different.
All backends in the tree support version 2 API but if you are using an out-of-tree backend, it must be adapted to use the new logging system.

Required action:
   Logging v1 uses the following three functions that must be replaced:

.. code-block::

   /* DEPRECATED! Functions used for logging v1. */
   void (*put)(const struct log_backend *const backend,
            struct log_msg *msg);
   void (*put_sync_string)(const struct log_backend *const backend,
            struct log_msg_ids src_level, uint32_t timestamp,
            const char *fmt, va_list ap);
   void (*put_sync_hexdump)(const struct log_backend *const backend,
            struct log_msg_ids src_level, uint32_t timestamp,
            const char *metadata, const uint8_t *data, uint32_t len);

Replace these functions with the following function used by logging v2:

.. code-block::

   /* Logging v2 function. */
   void (*process)(const struct log_backend *const backend,
           union log_msg2_generic *msg);

Zephyr v3.x.x namespace change
******************************

All Zephyr public headers have been moved to :file:`include/zephyr`, meaning they must be prefixed with ``<zephyr/...>`` when included.
Because this change can potentially break many applications or libraries, :kconfig:option:`CONFIG_LEGACY_INCLUDE_PATH` is provided to allow using the old include path.

.. note::
   The :kconfig:option:`CONFIG_LEGACY_INCLUDE_PATH` Kconfig option is disabled by default and will be removed soon.

In order to facilitate the migration to the new include prefix, a script to automate the process is also provided in :file:`scripts/utils/migrate_includes.py` (in Zephyr).

Changes in PWM API
******************

Zephyr v3.x.x introduces changes in the PWM API that require modifying the board definitions.
Old board definitions will cause a compilation error, and calling the old API functions will result in warnings stating that these functions are deprecated.

Required action:
   * ``pwms`` properties in devicetree nodes must be extended with two more cells (with period and flags) and now they need to specify PWM channels, not pin numbers.
   * Calls to the deprecated ``pwm_pin_set_cycles`` function must be replaced with calls to the :c:func:`pwm_set_cycles` function.
   * Calls to the deprecated ``pwm_pin_set_usec`` and ``pwm_pin_set_nsec`` functions must be replaced with calls to the :c:func:`pwm_set` function with the period and pulse values wrapped in the :c:macro:`PWM_USEC` macro or the :c:macro:`PWM_NSEC` macro, respectively.

Note that the :c:func:`pwm_set` and :c:func:`pwm_set_cycles` functions take a PWM channel as a parameter, not a pin number as the deprecated functions did.
Also, the ``flags`` parameter is now supported, so either the :c:macro:`PWM_POLARITY_INVERTED` or :c:macro:`PWM_POLARITY_NORMAL` flag must be provided in each call.

Wherever possible, it is recommended to use the newly introduced :c:macro:`PWM_DT_SPEC_GET` macro (or another suitable one from its family) to obtain PWM information from devicetree, and then use the :c:func:`pwm_set_dt` or :c:func:`pwm_set_pulse_dt` function instead of :c:func:`pwm_set`.

For example, for PWM channels defined as follows:

.. code-block:: devicetree

    pwm0_default: pwm0_default {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 11)>;
            nordic,invert;
        };
        group2 {
            psels = <NRF_PSEL(PWM_OUT3, 1, 5)>;
        };
    };

    pwm0_sleep: pwm0_sleep {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 11)>,
                    <NRF_PSEL(PWM_OUT3, 1, 5)>;
            low-power-enable;
        };
    };

    &pwm0 {
        status = "okay";
        pinctrl-0 = <&pwm0_default>;
        pinctrl-1 = <&pwm0_sleep>;
        pinctrl-names = "default", "sleep";
    };

You must update the PWM LED definitions that use those channels:

.. code-block:: devicetree

    /* old definitions that will no longer work */
    pwm_led0: pwm_led_0 {
        pwms = <&pwm0 11>;
    };
    pwm_led1: pwm_led_1 {
        pwms = <&pwm0 37>;
    };

The above PWM LED definitions must be updated in the following way:

.. code-block:: devicetree

    /* updated definitions */
    pwm_led0: pwm_led_0 {
        pwms = <&pwm0 0 PWM_MSEC(20) PWM_POLARITY_INVERTED>;
    };
    pwm_led1: pwm_led_1 {
        pwms = <&pwm0 3 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
    };

.. note::
   The period lengths, set here arbitrarily to commonly used value of 20 ms, are provided as default ones.
   They can be overridden in the actual PWM API calls if needed.

Then, you can use the definitions in PWM API calls in the following way:

.. code-block:: c

    #define PWM_LED0_NODE DT_NODELABEL(pwm_led0)
    #define PWM_LED1_NODE DT_NODELABEL(pwm_led1)
    static const struct pwm_dt_spec led0_spec = PWM_DT_SPEC_GET(PWM_LED0_NODE);
    static const struct pwm_dt_spec led1_spec = PWM_DT_SPEC_GET(PWM_LED1_NODE);
    /* ... */
    /* Use 10 ms period for LED0 to override the default 20 ms from devicetree. */
    ret = pwm_set_dt(&led0_spec, PWM_MSEC(10), PWM_USEC(pulse_us));
    /* ... */
    ret = pwm_set_pulse_dt(&led1_spec, PWM_USEC(pulse_us));
