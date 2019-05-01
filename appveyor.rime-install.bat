set plugin_dir=plugins\librime-lua
mklink /j \projects\librime\%plugin_dir% .
cd \projects\librime
call %plugin_dir%\appveyor.install.bat
.\appveyor.install.bat
