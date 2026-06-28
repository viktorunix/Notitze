# 📝 Notitze

**Notitze** is a lightweight, note-taking application with very basic drawing capabilities built from scratch in **C** using **Raylib** as the graphics library.

## 💡 Why?

> This project started as a frustration I had when I couldn't find a good note-taking program on Windows that doesn't have a paywall, subscription, or a plan to integrate such payment plans for basic features. So I decided to create one that is lightweight, portable, and compatible with both **Windows AND Linux**.

## ✨ Key Features

* 🖌️ **Basic Tools:** Pen, Highlighter, Pencil (experimental), Line, Rectangle, Circle, and Eraser.

* 🎛️ **Hardware Pressure Sensitivity:** Made exclusively for handwritten note-taking with pressure sensitivity support! *(Note for Linux: you have to run the program as root to use pressure sensitivity because the input is drawn directly from `/dev/input/` and not through X11 or Wayland).*

* 〰️ **Basic Stroke Smoothing:** Enjoy clean and smooth handwriting interpolation.

* 📚 **Notebook Organization:** Notes are organized into a single database with the option of exporting/importing external files.

* 📄 **Infinite Pages:** Add as many pages as you need, completely customized (background and size).

* 🥞 **Layers:** Full layer support, 'cause why not!

* 🚀 **Render Modes:** Choose between Baked or Live rendering, 'cause why not!

## 🛠️ Building from Source

### Prerequisites

* **GCC / MinGW**

* **Make**

* **Raylib library** (`.dll` for Windows or `.a` for Linux, placed into the `src/lib` folder)

### Windows Compilation

```
make all

```

### Linux Compilation

```
make -f Makefile.linux
```
