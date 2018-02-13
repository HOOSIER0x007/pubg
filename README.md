# Credit
@KN4CK3R created this tool, I merely adapted it to PUBG.
Please check out [his repo](https://github.com/KN4CK3R) to get an SDK generator for other games.

## How to Use:

* Compile all files in the src directory (x64 Release).
* Inject the resultant DLL into PUBG.
* Wait for the magic to happen.
* Click 'OK' on the Finished message box.
* Copy pasta SDK into your cheat.
* Fix UTimelineComponent in AParachutePlayer_C. (PUBG_ParachutePlayer_classes.hpp)
* Add AActors and AActorsForGC right before OwningWorld in ULevel. (PUBG_Engine_classes.hpp)
* Add Location and Bounds to USceneComponent right after PhysicsVolume. (PUBG_Engine_classes.hpp)
* Compile.

## Notes
* Default Directory is D:/SDK_GEN
* It is not perfectly aligned!
* If you can improve the alignment, please make a pull request.