#define CPPHTTPLIB_OPENSSL_SUPPORT 0
#include <httplib.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static const char* HOST       = "cppmiltech.com.ua";
static const int   PORT       = 80;
static const char* API_KEY    = "dz12-vX7mK4qT9r2w";
static const char* STUDENT_ID = "2042";
static const int   MAX_TRIES  = 5;

static const char* TESTS[] = {
    "T01","T02","T03","T04","T05","T06","T07","T08","T09","T10"
};

struct Rec { const char* id; int tries; int status; };

static std::string slurp(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::ostringstream s;
    s << f.rdbuf();
    return s.str();
}

static bool ok2xx(int s) { return s >= 200 && s < 300; }
static bool retryable(int s) { return s == 503 || s == 0; }

static httplib::Client make_cli() {
    httplib::Client cli(HOST, PORT);
    cli.set_connection_timeout(2);
    cli.set_read_timeout(2);
    return cli;
}

int main(int argc, char** argv) {
    const char* dir = argc > 1 ? argv[1] : "data";
    std::vector<Rec> recs;

    for (auto tid : TESTS) {
        std::string path = std::string(dir) + "/" + tid + "/simulation.json";
        std::string sim = slurp(path);
        if (sim.empty()) {
            fprintf(stderr, "[%s] skip: no file\n", tid);
            recs.push_back({tid, 0, -1});
            continue;
        }

        std::string body = std::string(R"({"studentId":")") + STUDENT_ID +
                           R"(","testId":")" + tid +
                           R"(","simulation":)" + sim + "}";

        httplib::Headers hdrs = {
            {"x-api-key", API_KEY},
            {"Content-Type", "application/json"}
        };

        int tries = 0, st = 0;
        while (tries < MAX_TRIES) {
            if (tries) std::this_thread::sleep_for(std::chrono::seconds(1));
            tries++;

            auto cli = make_cli();
            auto r = cli.Post("/api/dz12/results", hdrs, body, "application/json");
            st = r ? r->status : 0;
            fprintf(stderr, "[%s] try %d/%d -> %d\n", tid, tries, MAX_TRIES, st);

            if (ok2xx(st))     break;
            if (!retryable(st)) break;
        }

        if (ok2xx(st)) {
            auto cli = make_cli();
            std::string gpath = std::string("/api/dz12/results/") + tid + "/" + STUDENT_ID;
            auto r = cli.Get(gpath.c_str(), {{"x-api-key", API_KEY}});
            fprintf(stderr, "[%s] verify -> %d\n", tid, r ? r->status : 0);
        }

        recs.push_back({tid, tries, st});
    }

    printf("\n%-6s %-12s %s\n", "test", "status", "tries");
    for (auto& rec : recs) {
        const char* label = rec.tries == 0 ? "skipped"
                          : ok2xx(rec.status) ? "ok"
                          : "failed";
        printf("%-6s %-12s %d\n", rec.id, label, rec.tries);
    }
    return 0;
}
