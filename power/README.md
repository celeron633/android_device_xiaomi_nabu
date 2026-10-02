# Nabu power profiles

Settings > Battery > Power profile provides three profiles. The default is
Balanced. Selection is stored in `persist.sys.nabu.power_profile` and restored
by init after boot, a Power HAL restart, or a change to the property.

| Profile | Little CPU cap | Big CPU cap | Prime CPU cap | GPU cap |
| --- | --- | --- | --- | --- |
| Power saving | 1.3824 GHz | 2.016 GHz | 2.016 GHz | 585 MHz |
| Balanced | 1.7856 GHz | 2.4192 GHz | 2.4192 GHz | 675 MHz |
| Performance | 1.7856 GHz | 2.4192 GHz | 2.9568 GHz | 675 MHz |

The app writes the selection property. A small system service applies the
corresponding named hint through the existing Power HAL's IPowerExt Binder
extension. The app does not write CPU/GPU sysfs nodes itself.

Profiles set maximum frequencies, leaving frequency governors and thermal
management enabled. Power saving also ends and masks LAUNCH and
EXPENSIVE_RENDERING boosts. Android Battery Saver applies the same caps through
LOW_POWER, including when Performance is selected. Ending either saving hint
removes only its own requests and masks.

libperfmgr resolves competing requests by value index. Maximum-frequency node
values are ordered from lowest to highest so launch boosts cannot override
profile, LOW_POWER, or sustained-performance limits. Original default node
values are preserved.

After flashing, check the selected profile and active HAL requests:

```sh
adb shell getprop persist.sys.nabu.power_profile
adb shell dumpsys android.hardware.power.IPower/default
adb logcat -s nabu-power-profiles
```

Switch through all three profiles, enable and disable Battery Saver while in
Performance, and reboot to check persistence. The limits are configuration
targets; actual clocks also depend on load and temperature. Static checks cover
profile/boost arbitration and XML/JSON consistency. Device behavior needs to be
verified after flashing.
