#pragma once

#include <memory>

#include "car_io.h"
#include "hingy_track.h"
#include "main.h"
#include "pid_controller.h"

class Driver
{
  public:
    Driver(stringmap params){};
    virtual ~Driver(){};

    virtual void Cycle(CarSteers &steers, const CarState &state) = 0;
    virtual stringmap GetSimulatorInitParameters() = 0;
};

class HingyDriver : public Driver
{
  private:
    std::shared_ptr<HingyTrack> track;
    float last_timestamp = 0.0f, speed_factor, speed_base;
    float last_dt = 0.0f, last_rpm = 0.0f;
    float master_output_factor, steering_factor;
    int gear_dir;
    float stuck_counter;
    bool steering_enabled = false;

    std::vector<float> grn_inputs;

    PidController cross_position_control;
    PidController angle_control;
    PidController speed_control;

    void SetClutchAndGear(const CarState &state, CarSteers &steers);
    void SetReverseGear(const CarState &state, CarSteers &steers);
    void StuckOverride(CarSteers &steers, const CarState &state, float dt);

    float GetTargetSpeed(const CarState &state);

  public:
    HingyDriver(stringmap params);
    virtual ~HingyDriver();

    virtual void Cycle(CarSteers &steers, const CarState &state);
    virtual stringmap GetSimulatorInitParameters();
};
