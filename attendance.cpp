/*
  STUDENT ATTENDANCE MANAGEMENT SYSTEM
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sys/stat.h>
#include <direct.h>

using namespace std;
using namespace cv;

const string CSV_FILE   = "attendance.csv";
const string PHOTOS_DIR = "attendance_photos";

//  Record
struct Record {
    string roll_no, date, photo_path;
};

//  Utils
class Utils {
public:
    static string date_string() {
        time_t now = time(nullptr);
        struct tm t{};
        localtime_s(&t, &now);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
        return buf;
    }

    static string datetime_string() {
        time_t now = time(nullptr);
        struct tm t{};
        localtime_s(&t, &now);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &t);
        return buf;
    }

    static bool dir_exists(const string &path) {
        struct stat info;
        return stat(path.c_str(), &info) == 0 && (info.st_mode & _S_IFDIR);
    }

    static void ensure_dir(const string &path) {
        if (!dir_exists(path)) _mkdir(path.c_str());
    }

    static void color(int c) { cout << "\033[" << c << "m"; }
    static void reset()      { cout << "\033[0m"; }
};

//  CSV
class CSVManager {
public:
    void init() {
        ifstream f(CSV_FILE);
        if (!f.good()) {
            ofstream o(CSV_FILE);
            o << "RollNo,Date,PhotoPath\n";
        }
    }

    bool already_today(const string &roll, const string &date) {
        ifstream f(CSV_FILE);
        string line;
        getline(f, line);
        while (getline(f, line)) {
            istringstream ss(line);
            string r, d;
            getline(ss, r, ',');
            getline(ss, d, ',');
            if (r == roll && d == date) return true;
        }
        return false;
    }

    void save(const Record &rec) {
        ofstream f(CSV_FILE, ios::app);
        f << rec.roll_no << "," << rec.date << "," << rec.photo_path << "\n";
    }

    vector<Record> load_all() {
        vector<Record> recs;
        ifstream f(CSV_FILE);
        string line;
        getline(f, line);
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Record r;
            getline(ss, r.roll_no,    ',');
            getline(ss, r.date,       ',');
            getline(ss, r.photo_path, ',');
            recs.push_back(r);
        }
        return recs;
    }
};

//  Camera
class Camera {
public:
    string capture(const string &roll_no) {
        VideoCapture cap;
        for (int idx : {0, 1}) {
            cap.open(idx, CAP_DSHOW);
            if (cap.isOpened()) break;
        }

        if (!cap.isOpened()) {
            Utils::color(31);
            cout << "\n  [!] No webcam found. Attendance recorded without photo.\n";
            Utils::reset();
            return "NO_PHOTO";
        }

        cap.set(CAP_PROP_FRAME_WIDTH,  1280);
        cap.set(CAP_PROP_FRAME_HEIGHT,  720);
        cap.set(CAP_PROP_BUFFERSIZE,     1);

        const string WIN = "Attendance Capture  |  SPACE = capture now   ESC = cancel";
        namedWindow(WIN, WINDOW_NORMAL);
        resizeWindow(WIN, 900, 560);

        cout << "\n  [*] Camera ready. Auto-captures in 3 seconds.\n";
        cout << "      SPACE = capture now   |   ESC = cancel\n";

        auto start = chrono::steady_clock::now();
        const double COUNTDOWN = 3.0;
        string saved_path;

        while (true) {
            Mat frame;
            cap >> frame;
            if (frame.empty()) { waitKey(15); continue; }

            flip(frame, frame, 1);

            double elapsed  = chrono::duration<double>(chrono::steady_clock::now() - start).count();
            int remaining   = (int)ceil(COUNTDOWN - elapsed);

            Mat disp = frame.clone();
            rectangle(disp, {0, disp.rows - 90}, {disp.cols, disp.rows}, {15, 15, 15}, FILLED);
            addWeighted(disp, 0.55, frame, 0.45, 0, disp);

            putText(disp, "Roll No: " + roll_no, {24, disp.rows - 55},
                    FONT_HERSHEY_DUPLEX, 0.85, {255, 255, 255}, 2, LINE_AA);

            string cnt_txt  = remaining > 0 ? "Capturing in " + to_string(remaining) + "s ..." : "  SMILE!  ";
            Scalar cnt_color = remaining > 0 ? Scalar(0, 210, 255) : Scalar(60, 255, 60);
            putText(disp, cnt_txt, {disp.cols - 340, disp.rows - 55},
                    FONT_HERSHEY_DUPLEX, 0.85, cnt_color, 2, LINE_AA);

            ellipse(disp, {disp.cols / 2, disp.rows / 2 - 20}, {115, 145},
                    0, 0, 360, {0, 255, 180}, 2, LINE_AA);

            int bar_w = (int)(disp.cols * min(1.0, elapsed / COUNTDOWN));
            rectangle(disp, {0, disp.rows - 6}, {bar_w, disp.rows}, {0, 200, 255}, FILLED);

            imshow(WIN, disp);
            int key = waitKey(20);

            if (key == 27) {
                Utils::color(33);
                cout << "\n  [!] Capture cancelled.\n";
                Utils::reset();
                saved_path = "NO_PHOTO";
                break;
            }

            if (remaining <= 0 || key == 32) {
                cap >> frame;
                if (!frame.empty()) flip(frame, frame, 1);

                Utils::ensure_dir(PHOTOS_DIR);
                saved_path = PHOTOS_DIR + "/" + roll_no + "_" + Utils::datetime_string() + ".jpg";

                vector<int> params = {IMWRITE_JPEG_QUALITY, 92};
                if (imwrite(saved_path, frame.empty() ? disp : frame, params)) {
                    Mat flash(disp.size(), disp.type(), {255, 255, 255});
                    imshow(WIN, flash); waitKey(120);
                    Utils::color(32);
                    cout << "\n  [+] Photo saved: " << saved_path << "\n";
                    Utils::reset();
                } else {
                    Utils::color(31);
                    cout << "\n  [!] Could not save photo.\n";
                    Utils::reset();
                    saved_path = "SAVE_FAILED";
                }
                break;
            }
        }

        cap.release();
        destroyAllWindows();
        return saved_path;
    }
};

//  AttendanceSystem

class AttendanceSystem {
    CSVManager csv;
    Camera camera;

public:
    AttendanceSystem() { csv.init(); }

    void mark_attendance() {
        Utils::color(36);
        cout << "\n  +-------------------------------+\n"
                "  |       MARK ATTENDANCE         |\n"
                "  +-------------------------------+\n";
        Utils::reset();

        string roll;
        cout << "  Enter Roll No: ";
        cin  >> roll;
        transform(roll.begin(), roll.end(), roll.begin(), ::toupper);

        string today = Utils::date_string();

        if (csv.already_today(roll, today)) {
            Utils::color(33);
            cout << "\n  [!] " << roll << " already marked today (" << today << ").\n";
            Utils::reset();
            return;
        }

        string photo = camera.capture(roll);
        csv.save({roll, today, photo});

        Utils::color(32);
        cout << "\n  +------------------------------------------+\n"
                "  |   Attendance Marked Successfully!         |\n"
                "  +------------------------------------------+\n";
        Utils::reset();
        cout << "     Roll No  : " << roll  << "\n"
             << "     Date     : " << today << "\n"
             << "     Photo    : " << photo << "\n";
    }

    void view_attendance() {
        Utils::color(36);
        cout << "\n  +-------------------------------+\n"
                "  |     VIEW ATTENDANCE %         |\n"
                "  +-------------------------------+\n";
        Utils::reset();

        cout << "  Enter Roll No (or 'all'): ";
        string roll;
        cin  >> roll;
        transform(roll.begin(), roll.end(), roll.begin(), ::toupper);

        auto recs = csv.load_all();
        if (recs.empty()) {
            Utils::color(33);
            cout << "\n  [!] No records found.\n";
            Utils::reset();
            return;
        }

        vector<string> dates;
        for (auto &r : recs)
            if (find(dates.begin(), dates.end(), r.date) == dates.end())
                dates.push_back(r.date);
        int total = (int)dates.size();

        auto print_one = [&](const string &rn) {
            int present = 0;
            vector<Record> srecs;
            for (auto &r : recs)
                if (r.roll_no == rn) { ++present; srecs.push_back(r); }

            if (srecs.empty()) {
                Utils::color(31);
                cout << "\n  [!] No records for: " << rn << "\n";
                Utils::reset();
                return;
            }

            double pct = total > 0 ? 100.0 * present / total : 0.0;
            int bar    = (int)(pct / 5.0);

            cout << "\n  +------------------------------------------+\n";
            cout << "  |  Roll No   : " << left << setw(28) << rn << "|\n";
            cout << "  |  Present   : " << setw(28)
                 << (to_string(present) + " / " + to_string(total) + " days") << "|\n";
            cout << "  |  Percentage: " << setw(28) << (to_string((int)pct) + "%") << "|\n";
            cout << "  +------------------------------------------+\n  [";

            Utils::color(pct >= 75 ? 32 : 31);
            for (int i = 0; i < 20; ++i) cout << (i < bar ? "#" : "-");
            Utils::reset();
            cout << "] " << fixed << setprecision(1) << pct << "%\n";

            if (pct < 75.0) { Utils::color(31); cout << "  WARNING: Below 75%!\n"; }
            else             { Utils::color(32); cout << "  Attendance satisfactory.\n"; }
            Utils::reset();

            cout << "\n  " << left << setw(14) << "Date" << setw(10) << "Status" << "Photo\n";
            cout << "  " << string(65, '-') << "\n";
            for (auto &s : srecs) {
                string p = s.photo_path.size() > 40
                    ? "..." + s.photo_path.substr(s.photo_path.size() - 37)
                    : s.photo_path;
                cout << "  " << setw(14) << s.date << setw(10) << "Present" << p << "\n";
            }
        };

        if (roll == "ALL") {
            vector<string> students;
            for (auto &r : recs)
                if (find(students.begin(), students.end(), r.roll_no) == students.end())
                    students.push_back(r.roll_no);
            for (auto &s : students) print_one(s);
        } else {
            print_one(roll);
        }
    }

    void view_today() {
        string today = Utils::date_string();
        Utils::color(36);
        cout << "\n  +------------------------------------------+\n";
        cout <<   "  |  TODAY'S ATTENDANCE  (" << today << ")  |\n";
        cout <<   "  +------------------------------------------+\n";
        Utils::reset();

        auto recs = csv.load_all();
        vector<Record> today_recs;
        for (auto &r : recs)
            if (r.date == today) today_recs.push_back(r);

        if (today_recs.empty()) {
            Utils::color(33); cout << "  No one marked today yet.\n"; Utils::reset();
            return;
        }

        cout << "\n  " << left << setw(14) << "Roll No" << "Photo\n";
        cout << "  " << string(60, '-') << "\n";
        for (auto &r : today_recs) {
            string p = r.photo_path.size() > 44
                ? "..." + r.photo_path.substr(r.photo_path.size() - 41)
                : r.photo_path;
            cout << "  " << setw(14) << r.roll_no << p << "\n";
        }
        Utils::color(32);
        cout << "\n  Total present: " << today_recs.size() << "\n";
        Utils::reset();
    }

    void view_photo() {
        Utils::color(36);
        cout << "\n  +-------------------------------+\n"
                "  |      VIEW STUDENT PHOTO       |\n"
                "  +-------------------------------+\n";
        Utils::reset();

        string roll;
        cout << "  Enter Roll No: ";
        cin  >> roll;
        transform(roll.begin(), roll.end(), roll.begin(), ::toupper);

        auto recs = csv.load_all();
        vector<Record> srecs;
        for (auto &r : recs)
            if (r.roll_no == roll && r.photo_path != "NO_PHOTO" && r.photo_path != "SAVE_FAILED")
                srecs.push_back(r);

        if (srecs.empty()) {
            Utils::color(33); cout << "  [!] No photos for: " << roll << "\n"; Utils::reset();
            return;
        }

        Mat img = imread(srecs.back().photo_path);
        if (img.empty()) {
            Utils::color(31);
            cout << "  [!] Cannot open: " << srecs.back().photo_path << "\n";
            Utils::reset();
            return;
        }

        putText(img, "Roll: " + srecs.back().roll_no + "   Date: " + srecs.back().date,
                {20, img.rows - 20}, FONT_HERSHEY_DUPLEX, 0.75, {0, 255, 180}, 2, LINE_AA);

        string win = "Photo: " + roll + " (" + srecs.back().date + ")  — press any key";
        namedWindow(win, WINDOW_NORMAL);
        resizeWindow(win, 800, 600);
        imshow(win, img);
        waitKey(0);
        destroyAllWindows();
    }

    void run() {
        Utils::ensure_dir(PHOTOS_DIR);

        Utils::color(36);
        cout << R"(
  +==========================================+
  |  STUDENT ATTENDANCE MANAGEMENT SYSTEM   |
  |                |
  +==========================================+
)";
        Utils::reset();

        while (true) {
            cout << "\n  +----------------------------------+\n"
                    "  |           MAIN MENU              |\n"
                    "  +----------------------------------+\n"
                    "  |  1. Mark Attendance              |\n"
                    "  |  2. View Attendance Percentage   |\n"
                    "  |  3. Today's Present List         |\n"
                    "  |  4. View Student Photo           |\n"
                    "  |  5. Exit                         |\n"
                    "  +----------------------------------+\n"
                    "  Choice: ";

            int ch;
            if (!(cin >> ch)) {
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (ch) {
                case 1: mark_attendance(); break;
                case 2: view_attendance(); break;
                case 3: view_today();      break;
                case 4: view_photo();      break;
                case 5:
                    Utils::color(32);
                    cout << "\n  Goodbye!\n\n";
                    Utils::reset();
                    return;
                default:
                    Utils::color(33);
                    cout << "\n  [!] Enter 1-5.\n";
                    Utils::reset();
            }
        }
    }
};

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    system("chcp 65001 >nul");
    system("");

    AttendanceSystem app;
    app.run();
    return 0;
}
