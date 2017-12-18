if exist "Builds\" (
	rmdir /S /Q "Builds"	
)

mkdir "Builds"

C:\DevelopmentTools\arduino-nightly\arduino --pref build.path=Builds --verify LCSController/LCSController.ino

PAUSE
