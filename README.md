# ATTENDANCE-MANAGEMENT-SYSTEM-
A C++ console application for managing student attendance with webcam photo capture. Attendance records are stored in a CSV file alongside timestamped photos of each student at the time of marking.

---

## Features

- **Mark Attendance** — Captures a live webcam photo with a countdown timer; falls back gracefully if no camera is available
- **View Attendance Percentage** — Displays present/total days and a visual progress bar with a 75% threshold warning; supports individual or all-student views
- **Today's Present List** — Shows all students who have been marked present on the current date
- **View Student Photo** — Opens the most recent captured photo for a given roll number in an OpenCV window
- **Duplicate Prevention** — A student cannot be marked twice on the same date

---

## Requirements

| Dependency | Purpose |
|---|---|
| C++17 (or later) | Language standard |
| [OpenCV 4.x](https://opencv.org/) | Camera capture and image display |
| Windows OS | Uses `_mkdir`, `localtime_s`, `CAP_DSHOW` |

---

## Building

### With CMake

```cmake
find_package(OpenCV REQUIRED)
target_link_libraries(attendance_system ${OpenCV_LIBS})
```

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### With g++ (MinGW)

```bash
g++ -std=c++17 -o attendance main.cpp `pkg-config --cflags --libs opencv4`
```

---

## Running

```bash
./attendance_system.exe
```

On launch, the program creates:
- `attendance.csv` — the attendance record file
- `attendance_photos/` — directory for captured student photos

---

## Usage

```
+----------------------------------+
|           MAIN MENU              |
+----------------------------------+
|  1. Mark Attendance              |
|  2. View Attendance Percentage   |
|  3. Today's Present List         |
|  4. View Student Photo           |
|  5. Exit                         |
+----------------------------------+
```

### Mark Attendance (Option 1)
1. Enter the student's roll number (case-insensitive).
2. A webcam window opens with a **3-second countdown** and an auto-capture.
3. Press `SPACE` to capture immediately or `ESC` to cancel and record without a photo.
4. The record is saved to `attendance.csv`.

### View Attendance Percentage (Option 2)
- Enter a roll number to view that student's stats, or type `all` to view every student.
- Output includes present/total days, a percentage bar, and a per-date breakdown.
- Students below 75% attendance receive a warning.

### Today's Present List (Option 3)
- Displays all roll numbers marked present today along with their photo paths.

### View Student Photo (Option 4)
- Enter a roll number to open the most recently captured photo in a resizable window.
- Press any key to close the window.

---

## Data Format

**`attendance.csv`**

```
RollNo,Date,PhotoPath
CS101,2025-06-10,attendance_photos/CS101_2025-06-10_09-15-32.jpg
CS102,2025-06-10,NO_PHOTO
```

| Field | Description |
|---|---|
| `RollNo` | Student roll number (uppercased) |
| `Date` | Date in `YYYY-MM-DD` format |
| `PhotoPath` | Relative path to the photo, `NO_PHOTO`, or `SAVE_FAILED` |

---

## Project Structure

```
attendance_system/
├── main.cpp                  # Full source code
├── attendance.csv            # Auto-created on first run
└── attendance_photos/        # Auto-created; holds captured JPEGs
```

---

## Known Limitations

- **Windows only** — Uses `_mkdir`, `localtime_s`, and DirectShow (`CAP_DSHOW`). Porting to Linux/macOS requires replacing these with POSIX equivalents.
- **No student roster** — Any roll number is accepted; there is no validation against a predefined list.
- **No absent tracking** — Only present records are stored; the total day count is derived from unique dates in the CSV.
- **Single-camera support** — Tries indices `0` and `1`; additional cameras are not enumerated.

---

## License

This project is provided as-is for educational purposes. No license is explicitly stated.
