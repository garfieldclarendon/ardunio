if exist "Builds\" (
	rmdir /S /Q "Builds"	
)

mkdir "Builds"
mkdir "Builds\BlockController"
mkdir "Builds\PanelController"
mkdir "Builds\SemaphoreController"
mkdir "Builds\SignalController"
mkdir "Builds\TurnoutController"
mkdir "Builds\MultiModuleController"

C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/BlockController --verify BlockController/BlockController.ino
C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/PanelController --verify PanelController/PanelController.ino
C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/SemaphoreController --verify SemaphoreController/SemaphoreController.ino
C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/SignalController --verify SignalController/SignalController.ino
C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/TurnoutController --verify TurnoutController/TurnoutController.ino
C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds/MultiModuleController --verify MultiModuleController/MultiModuleController.ino

PAUSE
