# UICR-NRF52-READ-and-WRITE

This firmware is designed for the **nRF52-DK** and demonstrates how to store factory calibration data in the UICR (User Information Configuration Registers). The system utilizes a button press to trigger the write operation, and the stored data can be verified using external tools such as `nrfjprog`.

## Features

- **Initialization Sequence**: Upon startup, all four LEDs on the nRF52-DK blink twice as a greeting.
- **Button-Triggered UICR Write**: 
  - Pressing **Button 1** writes a 32-bit calibration value to the `CUSTOMER[0]` memory location (`0x10001080`) in the UICR.
  - The write operation includes:
    - Temperature offset.
    - Temperature setpoint.
    - Operation mode.
    - Sensor status.
  - The firmware writes the data in a pre-concatenated 32-bit format.
- **Debouncing Logic**: Ensures button presses are properly debounced to avoid multiple triggers during a single press.
- **System Reset**: After successfully writing to the UICR, the system automatically resets to apply the new configuration.

## UICR Overview

The **UICR (User Information Configuration Registers)** is a dedicated memory area in the nRF52 SoC that allows non-volatile storage of factory calibration data or other user-specific parameters. This firmware utilizes the `CUSTOMER[0]` section located at `0x10001080`.

### 32-Bit Data Structure
The stored data is organized as follows:

| **Bits** | **Field**            | **Description**                      |
|----------|----------------------|--------------------------------------|
| 0–7      | Temperature Offset   | Offset in °C (8-bit signed integer). |
| 8–9      | Reserved             | Reserved for future use.             |
| 10–11    | Operation Mode       | Operation mode (2-bit).              |
| 12–13    | Sensor Status        | Sensor status (2-bit).               |
| 14–23    | Temperature Setpoint | Setpoint in 0.1 °C units (10-bit).   |
| 24–31    | Reserved             | Reserved for future use.             |

### Example Data
For example, if the following values are written:
- **Offset Temperature**: 10°C
- **Setpoint Temperature**: 750 (represents 75.0 °C)
- **Mode**: 1
- **Status**: 0

The 32-bit data stored in `0x10001080` would be: `0x00BB840A`.

## Usage Instructions

### Hardware Setup
1. Connect the **nRF52-DK** to your PC via USB.
2. Ensure that **Button 1** and **LED 1–4** are functional.

### Firmware Behavior
1. On power-up or reset, the LEDs blink twice as a startup greeting.
2. Press **Button 1** to trigger the write operation to the UICR.
3. Upon successful write, the device resets automatically.

### Verification
Use the `nrfjprog` tool to verify the written data:
```bash
nrfjprog --memrd 0x10001080 --n 4

Expected Output:
0x10001080: 00BB840A                              |....|
