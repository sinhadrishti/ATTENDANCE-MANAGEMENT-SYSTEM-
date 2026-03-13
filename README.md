# 📋 Face Detection Attendance Management System

A console-based C++ application for tracking student attendance with webcam photo capture, CSV record storage, and attendance analytics.

---

## 📁 Project Overview

| Property | Details |
|---|---|
| **Language** | C++ (C++14 / C++17) |
| **Total Lines of Code** | ~340 lines |
| **External Library** | OpenCV 4.x |
| **Storage** | CSV flat-file (`attendance.csv`) |
| **Platform** | Windows (uses `_mkdir`, `CAP_DSHOW`, `chcp`) |
| **Paradigm** | Object-Oriented Programming (OOP) |

---

## 🏗️ OOP Concepts Used

### 1. 🔷 Classes & Objects
The system is divided into **4 distinct classes**, each with a single responsibility:

| Class | Responsibility |
|---|---|
| `Utils` | Static helper utilities (dates, colors, directories) |
| `CSVManager` | All CSV read/write operations |
| `Camera` | Webcam capture and photo saving |
| `AttendanceSystem` | Main orchestrator — holds the menu loop and coordinates all other classes |

Each class is instantiated as an **object** within `AttendanceSystem`, cleanly separating concerns.

---

### 2. 🔷 Encapsulation
Every class **encapsulates** its own data and logic:

- `CSVManager` manages only file I/O — callers never deal with raw file streams.
- `Camera` owns the entire webcam lifecycle (open → capture → release → close window).
- `AttendanceSystem` holds `CSVManager csv` and `Camera camera` as **private member objects**, exposing only high-level public methods.

```cpp
class AttendanceSystem {
    CSVManager csv;   // private — hidden from outside
    Camera camera;    // private — hidden from outside
public:
    void mark_attendance();
    void view_attendance();
    ...
};
```

---

### 3. 🔷 Abstraction
Implementation details are hidden behind clean interfaces:

- Calling `csv.save(record)` doesn't expose how the file is opened or written.
- Calling `camera.capture(roll_no)` returns just a file path string — the caller doesn't manage any `VideoCapture` object.
- `Utils::date_string()` returns a formatted date without exposing `time_t` / `struct tm` to callers.

---

### 4. 🔷 Structs as Data Containers
```cpp
struct Record {
    string roll_no, date, photo_path;
};
```
The `Record` struct acts as a **Plain Old Data (POD)** type — a lightweight aggregate to pass attendance data cleanly between classes without coupling logic to storage format.

---

### 5. 🔷 Static Methods (Utility Pattern)
`Utils` uses **static methods** — no instantiation needed. This is a classic OOP utility/helper pattern:

```cpp
Utils::date_string();      // returns "2025-07-15"
Utils::datetime_string();  // returns "2025-07-15_10-30-00"
Utils::ensure_dir(path);   // creates directory if absent
Utils::color(32);          // sets terminal color (green)
Utils::reset();            // resets terminal color
```

---

### 6. 🔷 Lambda Functions (Modern C++)
Inside `view_attendance()`, a **lambda** is used to print one student's report — capturing outer variables by reference, avoiding code duplication without needing a separate named function:

```cpp
auto print_one = [&](const string &rn) {
    // access recs, total, etc. from outer scope
    ...
};
```

---

### 7. 🔷 Constructor Initialization
`AttendanceSystem` uses its **constructor** to initialize the CSV file on startup:

```cpp
AttendanceSystem() { csv.init(); }
```
This ensures the CSV header exists before any operations are performed.

---

## ⚙️ C++ Features & STL Used

| Feature | Where Used | Purpose |
|---|---|---|
| `std::vector<Record>` | `CSVManager::load_all()` | Stores all attendance records in memory |
| `std::vector<string>` | `view_attendance()` | Collects unique dates and student roll numbers |
| `std::ifstream` / `std::ofstream` | `CSVManager` | File reading and appending |
| `std::istringstream` | `CSVManager::load_all()` | CSV line parsing |
| `std::chrono` | `Camera::capture()` | Countdown timer for auto-capture |
| `std::transform` | `mark_attendance()`, `view_attendance()` | Converts roll number input to uppercase |
| `std::find` | `view_attendance()` | Deduplicates date and student lists |
| `std::fixed`, `std::setprecision` | `view_attendance()` | Formats percentage output |
| `std::setw`, `std::left` | Table-style console output | Aligns columns in attendance reports |
| `time_t`, `struct tm`, `strftime` | `Utils` | Date/time formatting |
| Lambda `[&](...)` | `view_attendance()` | Inline per-student report printer |
| Range-based `for` | Throughout | Clean iteration over vectors |

---

## ✨ Features

### 1. Mark Attendance
- Enter a roll number (auto-converted to uppercase).
- Checks for **duplicate entries** — a student cannot be marked twice on the same day.
- Opens the webcam with a **live preview window**.
- Displays a **3-second countdown** with a progress bar and face-guide ellipse overlay.
- Supports manual capture via **SPACE** or cancellation via **ESC**.
- Saves a timestamped JPEG photo to the `attendance_photos/` folder.
- Writes the record (`RollNo, Date, PhotoPath`) to `attendance.csv`.

### 2. View Attendance Percentage
- Enter a specific roll number **or** type `all` to see every student.
- Displays:
  - Total present / total class days
  - Attendance percentage
  - An ASCII progress bar (green if ≥ 75%, red if below)
  - A **WARNING** flag for students below 75%
  - A full date-by-date attendance log with photo paths

### 3. Today's Present List
- Shows a quick snapshot of every student marked **today**.
- Displays roll number and associated photo path.
- Shows total count of students present.

### 4. View Student Photo
- Enter a roll number to view the **most recent captured photo**.
- Opens a resizable OpenCV window with the photo.
- Overlays the student's roll number and date on the image.
- Press any key to close.

### 5. Graceful No-Camera Handling
- If no webcam is detected, attendance is still recorded with `photo_path = "NO_PHOTO"` — the system never crashes.

---

## 📂 File Structure

```
project/
│
├── main.cpp                  # All source code (~340 lines)
├── attendance.csv            # Auto-created on first run
│
└── attendance_photos/        # Auto-created on first run
    ├── CS101_2025-07-15_10-30-00.jpg
    ├── CS102_2025-07-15_10-31-45.jpg
    └── ...
```

---

## 🧩 Class Architecture Diagram

```
main()
  └── AttendanceSystem
        ├── CSVManager
        │     ├── init()           → Creates CSV with header if absent
        │     ├── already_today()  → Duplicate check
        │     ├── save()           → Appends a record
        │     └── load_all()       → Returns vector<Record>
        │
        ├── Camera
        │     └── capture()        → Opens webcam, returns saved photo path
        │
        └── Utils (static)
              ├── date_string()
              ├── datetime_string()
              ├── ensure_dir()
              ├── dir_exists()
              ├── color()
              └── reset()
```

---

## 🔧 Dependencies & Build

### Requirements
- **OpenCV 4.x** — for webcam access and image display
- **C++14 or later** — for `auto`, lambdas, range-based for, `std::chrono`
- **Windows** — uses `_mkdir` (direct.h) and `CAP_DSHOW` backend

### Compile (MinGW / g++)
```bash
g++ main.cpp -o attendance -lopencv_core -lopencv_videoio -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -std=c++14
```

### Compile (MSVC)
Link against OpenCV debug/release libs in project properties, then build normally.

---

## 📊 Lines of Code Breakdown

| Section | Approx. Lines |
|---|---|
| Includes & constants | 15 |
| `struct Record` | 5 |
| `class Utils` | 30 |
| `class CSVManager` | 55 |
| `class Camera` | 100 |
| `class AttendanceSystem` | 120 |
| `main()` | 8 |
| **Total** | **~340** |

---

## 📌 CSV Format

```
RollNo,Date,PhotoPath
CS101,2025-07-15,attendance_photos/CS101_2025-07-15_10-30-00.jpg
CS102,2025-07-15,NO_PHOTO
CS101,2025-07-16,attendance_photos/CS101_2025-07-16_09-15-22.jpg
```

---

## 🎓 Academic Context

This project demonstrates the following OOP and C++ concepts commonly assessed in undergraduate coursework:

- **Encapsulation** — private member data, public interfaces
- **Abstraction** — hiding implementation behind method calls
- **Struct vs Class** — using structs for data, classes for behavior
- **Static utility methods** — stateless helpers without instantiation
- **STL containers** — `vector`, `string`, `ifstream`, `ostringstream`
- **Lambda closures** — capturing scope for inline logic
- **RAII** — `VideoCapture` and file streams auto-close on scope exit
- **Chrono library** — high-resolution timing for countdown logic
