// Copyright (C) 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include <aidl/google/hardware/power/extension/pixel/IPowerExt.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_auto_utils.h>
#include <android/binder_manager.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

using aidl::google::hardware::power::extension::pixel::IPowerExt;

int main(int, char** argv) {
    android::base::InitLogging(argv);
    const std::string profile = android::base::GetProperty(
            "persist.sys.nabu.power_profile", "balanced");
    const std::string selected = profile == "power_save" ? "PROFILE_POWER_SAVE"
            : profile == "performance" ? "PROFILE_PERFORMANCE" : "PROFILE_BALANCED";

    // init can announce the service before its Binder instance is registered.
    ndk::SpAIBinder power;
    for (int attempt = 0; attempt < 100; ++attempt) {
        power = ndk::SpAIBinder(
                AServiceManager_checkService("android.hardware.power.IPower/default"));
        if (power.get() != nullptr) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ndk::SpAIBinder extension;
    if (power.get() == nullptr ||
            AIBinder_getExtension(power.get(), extension.getR()) != STATUS_OK) {
        LOG(ERROR) << "Power HAL extension is unavailable";
        return EXIT_FAILURE;
    }
    auto hal = IPowerExt::fromBinder(extension);
    if (hal == nullptr) {
        LOG(ERROR) << "Power HAL has no IPowerExt implementation";
        return EXIT_FAILURE;
    }

    constexpr std::array<const char*, 3> modes = {
        "PROFILE_POWER_SAVE", "PROFILE_BALANCED", "PROFILE_PERFORMANCE"};
    for (const char* mode : modes) {
        bool supported = false;
        if (!hal->isModeSupported(mode, &supported).isOk() || !supported) {
            LOG(ERROR) << "Unsupported power profile: " << mode;
            return EXIT_FAILURE;
        }
    }
    // Apply the new limit first so switching profiles never temporarily uncaps it.
    if (!hal->setMode(selected, true).isOk()) return EXIT_FAILURE;
    for (const char* mode : modes) {
        if (selected != mode && !hal->setMode(mode, false).isOk()) return EXIT_FAILURE;
    }
    LOG(INFO) << "Applied power profile: " << selected;
    return EXIT_SUCCESS;
}
