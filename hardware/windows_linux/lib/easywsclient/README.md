# easywsclient

A simple WebSocket client library for C++.

## Source

This library is vendored from: https://github.com/dhbaird/easywsclient

## License

MIT License (see LICENSE file)

## Usage in OMOTE Simulator

This library provides WebSocket client functionality for the Windows/Linux simulator,
enabling bidirectional communication with the Remote Control Hub over WebSockets.

The library is automatically included in the simulator build via platformio.ini.

## Files

- `easywsclient.hpp` - Header file with class definitions
- `easywsclient.cpp` - Implementation
- `LICENSE` - MIT license

## Integration

The library is integrated into the simulator through:
- `websocket_hal_windows_linux.cpp` - HAL implementation using easywsclient
- Build flags in `platformio.ini` to include the library path

