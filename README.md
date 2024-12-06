# gtk_ImageResizer

## Overview

**Image Resizer** is a GTK-based desktop application written in C for resizing multiple `.jpg` images in a specified directory. The program allows users to select an image folder, choose a resizing option, and process images in the background, displaying progress and logs in a user-friendly interface.

---

## Features

- **Folder Selection**: Browse and select a directory containing `.jpg` images.
- **Resize Options**: Choose from four predefined image sizes:
  - Photograph (Height: 1200px)
  - Article (Height: 600px)
  - Thumbnail (Height: 200px)
  - Icon (Height: 50px)
- **Real-time Progress**: Updates a progress bar as images are resized.
- **Log Output**: Displays resizing details in a scrollable text view.
- **Threaded Processing**: Runs image resizing in a separate thread to keep the UI responsive.

---

## Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. Install required dependencies:
   ```bash
   sudo apt-get install libgtk-3-dev libgdk-pixbuf2.0-dev
   ```

3. Compile the program or Install:
   
  compilation
   ```bash
   ./GCompileAndPack.sh
   ```
  install
   ```bash
   sudo dpkg -i ImageSegregator.deb
   ```

5. Run installed program:
   ```bash
   ImageSegregator
   ```

---

## Usage

1. Launch the application.
2. Click **Select Folder** to choose a directory containing `.jpg` images.
3. Select a resize option from the dropdown menu.
4. Click **Resize Images** to begin processing.
5. Monitor the progress bar and log for updates.

---

## File Management

- Resized images are saved in the same folder as the original, prefixed with `s_` (e.g., `original.jpg` → `s_original.jpg`).

---

## Dependencies

- **GTK+ 3.0**
- **GDK-Pixbuf 2.0**
- **POSIX Threads (pthread)**

---


## Links

My tools for images segregation, resizing etc:

* ![Image Decoupler GUI](https://github.com/marcin-filipiak/gtkmm_ImageDecoupler) - GTKmm, find duplicated images in subfolders
* ![Image Decoupler Console](https://github.com/marcin-filipiak/cpp_ImageDecoupler) - Console, find duplicated images in subfolders
* ![Image Segregator GUI](https://github.com/marcin-filipiak/gtk_ImageSegregator) - GTK, usefull tool for images segregation
* ![Image Resizer GUI](https://github.com/marcin-filipiak/gtk_ImageResizer) - GTK , resize images in folder
* ![Image Resizer GUI](https://github.com/marcin-filipiak/qt_ImgFolderResizer) - Qt, resize images in folder
