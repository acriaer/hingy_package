// GRNTrainer.cpp : Defines the entry point for the console application.
//

#include <array>
#include <fstream>
#include <sstream>
#include <string>

#include <memory>
#include <boost/algorithm/string.hpp>

#include "ParamsTrainer.h"

#ifdef _WIN64
#define popen _popen
#define pclose _pclose
#endif

using std::string;

const string tester_path = "../../TORCSTester/TORCSTester/";
const string tester_executable = "bin/Debug/TORCSTester.exe";
const string tester_profile = "cases/learn.xml";
const string tester_options = " 4";
const string hingybot_path = "../../hingybot/hingybot/";

const string individual_being_evaluated = "configs/current.xml";
const string individual_best = "configs/best.xml";
const string individual_initial = "configs/initial.xml";

float best = -100.0;

std::string exec() {
    std::array<char, 128> buffer; 
    std::string result;
    std::string tester_call = (tester_path + tester_executable + " " + tester_path + tester_profile + " " + tester_options);
    std::shared_ptr<FILE> pipe(popen(tester_call.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return result;
}

float ParamsFitness(std::shared_ptr<Trainable<float>> seeker) {
    float fitness;
    auto tmp_seeker = std::dynamic_pointer_cast<ParamSeeker>(seeker); //ugly af
    tmp_seeker->WriteParams(hingybot_path + individual_being_evaluated);
    string out = exec();

    vector<string> lines;
    boost::algorithm::split(lines, out, boost::is_any_of("\n"));

    for (auto& line : lines) {
        if (strncmp(line.c_str(), "Mean ref:", 7) == 0)
            sscanf(line.c_str(), "Mean ref: %f", &fitness);
    }

    if (-fitness > best) {
        best = -fitness;
        tmp_seeker->WriteParams(hingybot_path + individual_best);
	tmp_seeker->WriteParams(hingybot_path + individual_best + "_" + std::to_string(-fitness));
        printf("New best! %f\n", -fitness);
    }

    return -fitness;
}

int main()
{
    ParamSeeker seeker;
    seeker.ReadParams(hingybot_path + individual_initial);

    auto gen = Randomizer<float>(2);
    auto trainer = new Trainer<ParamSeeker>(36, 0, 0, 1.0f, 1.0f, ParamsFitness, seeker.end_params, gen) ;

    while (true) {
        printf("Gene %f\n", trainer->Generation(gen));
    }

    printf("%s", exec().c_str());
    return 0;
}

void log_error(std::string msg)
{
    fprintf(stderr, "[ERROR]   %s\n", msg.c_str());
    exit(1);
}

void log_warning(std::string msg)
{
    fprintf(stdout, "[WARNING] %s\n", msg.c_str());
}

void log_info(std::string msg)
{
    fprintf(stdout, "[INFO]\t  %s\n", msg.c_str());
}
