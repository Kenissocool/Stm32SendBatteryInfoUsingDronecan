# Stm32 Sending Battery Data using Dronecan

A minimal working implementation of the [DroneCAN](https://dronecan.github.io/) protocol using **libcanard** and the [Arduino-DroneCAN](https://github.com/BeyondRobotix/Arduino-DroneCAN) library for STM32 or other microcontrollers.

---

## 🚀 Overview

This project demonstrates how to:

* Integrate libcanard into a bare-metal STM32 project
* Use DroneCAN message definitions without manually compiling DSDL
* Transmit and receive DroneCAN messages

We use the `Arduino-DroneCAN` repository because it simplifies integration: DSDL files are already compiled to C/C++.

---

## 📦 Library Installation

### 1. Add the Submodule

Clone this repo or add the submodule manually:

```bash
git submodule add https://github.com/BeyondRobotix/Arduino-DroneCAN lib/Arduino-DroneCAN
```

### 2. Copy Required Folders

From `lib/Arduino-DroneCAN`, copy the following into your project:

* `libcanard/`
* `dronecan/`

You can organize them like this:

```
your-project/
├── Core/
├── lib/
│   ├── libcanard/
│   │   ├── canard.c
│   │   ├── canard.h
│   │   └── canard_internals.h
│   └── dronecan/
│       ├── inc/
│       └── src/
```

### 3. Add to Build

In your build system (Makefile or STM32CubeIDE):

* Add `libcanard/canard.c` to the build sources.
* Add `dronecan/src/*.cpp` if using C++, or extract `.c` files.
* Include paths:

  * `lib/libcanard/`
  * `lib/dronecan/inc/`

---

## 📚 File Overview

### libcanard

| File                 | Description                                                 |
| -------------------- | ----------------------------------------------------------- |
| `canard.c`           | Main source file. Add to your build.                        |
| `canard.h`           | Public API. Include this in your application code.          |
| `canard_internals.h` | Internal definitions. Normally no need to include manually. |

### dronecan

| Folder | Description                                 |
| ------ | ------------------------------------------- |
| `src/` | Source code for message encoding/decoding.  |
| `inc/` | Headers defining message types and helpers. |

---

## 🛠️ Usage Example

### 1. Setting node id

```c
canardSetLocalNodeID(&canard, 11);
```

### 2. Naming Node

```c
const char *name = "kuybat";
```


### 3. Sending battery data

```c
static void send_BatteryInfo(void)
{
    struct uavcan_equipment_power_BatteryInfo batt_info;
    memset(&batt_info, 0, sizeof(batt_info));


    batt_info.temperature = 383.5f;      // d? K
    batt_info.voltage = 25.2f;          // V
    batt_info.current = 2.1f;          // A
    batt_info.average_power_10sec = 200.0f; // W
    batt_info.remaining_capacity_wh = 40.0f;
    batt_info.full_charge_capacity_wh = 50.0f;
    batt_info.hours_to_full_charge = 0.5f;
    batt_info.status_flags = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_IN_USE;
    batt_info.state_of_health_pct = 98;
    batt_info.state_of_charge_pct = 80;
    batt_info.state_of_charge_pct_stdev = 1;
    batt_info.battery_id = 1;
    batt_info.model_instance_id = 0x12345678;

    batt_info.model_name.len = 8;
    memcpy(batt_info.model_name.data, "LiPo4s", 8);


    uint8_t buffer[UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE];
    uint32_t len = uavcan_equipment_power_BatteryInfo_encode(&batt_info, buffer
    #if CANARD_ENABLE_TAO_OPTION
        , true
    #endif
    );

    static uint8_t transfer_id = 0;
    canardBroadcast(&canard,
                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE,
                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}
```

---

## ⏱ Timing and Message Scheduling

* Call `canardTxPeek()` periodically (e.g. every 1–10 ms)
* Publish messages (like `NodeStatus`) at a fixed rate (e.g. 1 Hz)

Example timer callback:

```c
void HAL_TIM_PeriodElapsedCallback(...) {
    if (publish_1hz_flag) {
        publish_heartbeat();
    }
    send_pending_canard_transfers();
}
```

---

## 🧠 Tips

* Assign your node a unique `node_id`
* Use `canardTxPush()` to queue messages
* Use a circular buffer (like libcanard's default allocator) for memory
* Enable FDCAN or bxCAN in STM32
* Use filters to accept only relevant messages (optional but recommended)

---

## 📄 References

* [libcanard](https://github.com/UAVCAN/libcanard)
* [Arduino-DroneCAN](https://github.com/BeyondRobotix/Arduino-DroneCAN)
* [DroneCAN Guide](https://dronecan.github.io/)
* [UAVCAN Specification](https://uavcan.org/specification/)

---

## 🧰 License

This integration is based on the Apache 2.0 licensed libcanard and DroneCAN tools.

---

Feel free to fork or contribute improvements!
