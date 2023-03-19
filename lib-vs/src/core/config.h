#pragma once

#include <vector>
#include <string>

#include <wpi/json.h>

#include "visioncamera.h"
#include "calib.h"

#define FRC_CONFIG	"/boot/frc.json"
#ifndef NT_CLIENT_VERSION
#define NT_CLIENT_VERSION 4
#endif

bool loadJson(wpi::json&, const char* file);

// template<typename... tmpl_T>
// bool searchCalibs(const wpi::json&, CalibMap_<tmpl_T...>&);

bool initNT(const char* file = FRC_CONFIG);
bool initSimNT(const char* sim_ip);
bool createCameras(std::vector<VisionCamera>& cameras, const char* file = FRC_CONFIG);
bool createCameras(std::vector<VisionCamera>& cameras, CalibList calibrations, const char* file = FRC_CONFIG);


// Leagacy loader
/**
 * Reads a config json and creates appropriate VisionCameras in the supplied vector
 * @param cameras The output vector in which cameras will be created
 * @param file The path to the json, default is "/boot/frc.json"
 * @return false if there was an error
*/
bool readConfig(std::vector<VisionCamera>& cameras, const char* file = FRC_CONFIG);