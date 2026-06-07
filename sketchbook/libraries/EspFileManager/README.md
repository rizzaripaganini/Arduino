# EspFileManager

**EspFileManager** is a library for creating a web-based file management system on the ESP32 using the Async WebServer library. The library provides a responsive interface that allows users to upload, download, and delete files stored on the ESP32's filesystem.

## Features
- **File Management:** Upload, download, and delete files via a web interface.
- **Responsive Design:** Interface adapts to different screen sizes, providing a smooth experience on desktops, tablets, and smartphones.

## Installation

### Arduino IDE
1. Navigate to the library manager (Sketch > Include Library > Manage Libraries).
2. Search for "esp32-file-manager" and install it.

### PlatformIO
1. Add the library to your `platformio.ini`:
   ```ini
   lib_deps =
       https://github.com/arslan437/EspFileManager.git

## Usage

    #include <EspFileManager.h>

    void setup() {
    // Initialize file manager
    }

    void loop() {
    // Handle file manager tasks
    }

## Support

Creating meaningful content is my passion, but it’s also a labor of love that requires time, energy, and often a bit of coffee to keep the creativity flowing! By buying me a cup of coffee, you're not just fueling my caffeine habit—you're helping me continue to create, innovate, and bring fresh ideas to life. Your support means the world to me, and every contribution, big or small, makes a huge difference.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request on GitHub.

## License

This library is licensed under the AGPL-3.0 License. See LICENSE for more information.