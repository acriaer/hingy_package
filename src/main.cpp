
#include <assert.h>
#include <chrono>
#include <limits.h>
#include <memory>

#include "driver.h"
#include "main.h"
#include "torcs_integration.h"
#include "utils.h"

using std::string;
using namespace std::chrono;

const std::vector<string> launch_arguments = {"host", "port",  "stage",
                                              "gui",  "track", "params"};

const std::vector<std::pair<string, string>> default_params = {
    {"track", "tmp_track.xml"},
    {"gui", "0"},
    {"stage", "1"},
    {"force1", "0"},
    {"force2", "0"},
    {"hinges_iterations", "60000"},
    {"paranoid", "0"},
    {"host", "127.0.0.1"}};

bool crash_on_warning;

int main(int argc, char **argv)
{
    int cycles = 0;
    stringmap launch_params;
    parse_arguments("", ':', argc - 1, &argv[1], launch_params);

    if (launch_params.find("params") == launch_params.end() ||
        !load_params_from_xml(launch_params["params"], "hingybot_params",
                              launch_params))
        log_warning("Parameters couldn't be read from the xml!");

    for (auto &param : default_params)
        if (launch_params.find(param.first) == launch_params.end())
            launch_params[param.first] = param.second;

    crash_on_warning = std::stoi(launch_params["paranoid"]) != 0;

    auto driver = std::unique_ptr<HingyDriver>(new HingyDriver(launch_params));

    std::unique_ptr<SimIntegration> integration =
        std::unique_ptr<TorcsIntegration>(new TorcsIntegration(launch_params));

    log_info("Waiting for the simulator hookup...");
    auto car_state = integration->Begin(driver->GetSimulatorInitParameters());
    CarSteers car_steers;
    log_info("Starting the main loop!");

    auto block_start_time = high_resolution_clock::now();

    while (true)
    {
        driver->Cycle(car_steers, car_state);
        auto car_state = integration->Cycle(car_steers);

        auto time = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      time - block_start_time)
                      .count();
        if (ms > 1000)
        {
            log_info(std::to_string((float)cycles / ((float)ms / 1000.0f)) +
                     " Hz");
            cycles = 0;
            block_start_time = high_resolution_clock::now();
        }

        cycles += 1;
    }

    return 0;
}

void log_error(std::string msg)
{
    fprintf(stderr, "[ERROR]   %s\n", msg.c_str());
    exit(1);
}

void log_warning(std::string msg)
{
    fprintf(stderr, "[WARNING] %s\n", msg.c_str());
    if (crash_on_warning)
        log_error("Paranoid crash on warning!\n");
}

void log_info(std::string msg)
{
    fprintf(stdout, "[INFO]\t  %s\n", msg.c_str());
}
