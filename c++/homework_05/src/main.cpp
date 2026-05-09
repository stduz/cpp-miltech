#include "telemetry.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: telemetry_check <input_path>\n";
        return 1;
    }

    Frame frames[MAX_TELEMETRY_FRAMES];
    const int frame_count = read_frames(argv[1], frames, MAX_TELEMETRY_FRAMES);

    // fix: порожній файл не повинен призводити до краша в summarize
    if (frame_count == 0) {
        std::cerr << "error: no frames in input file\n";
        return 1;
    }

    const Summary summary = summarize(frames, frame_count);
    print_summary(summary);

    return 0;
}
