# Libcanard DroneCAN Integration

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

### 1. Initialize Canard Instance

```c
CanardInstance instance = canardInit(&mem_allocate, &mem_free);
instance.mtu_bytes = 64;
instance.node_id = 42; // Set your node ID
```

### 2. Send a Message (e.g. Heartbeat)

```c
uint8_t buffer[CANARD_MTU_MAX];
size_t payload_size = uavcan_protocol_NodeStatus_encode(&status_msg, buffer);

CanardTransfer xfer = {
    .priority = CanardPriorityNominal,
    .transfer_kind = CanardTransferKindMessage,
    .port_id = UAVCAN_PROTOCOL_NODESTATUS_ID,
    .remote_node_id = CANARD_BROADCAST_NODE_ID,
    .transfer_id = transfer_id++,
    .payload_size = payload_size,
    .payload = buffer
};

canardTxPush(&instance, &xfer);
```

### 3. Receive a Message

```c
const CanardFrame* rx_frame = canardRxAccept(&instance, &frame, &metadata);
if (rx_frame) {
    // Check message type, port ID, etc.
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
