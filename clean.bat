cd DbViewer

rmdir /S /Q Debug Release
del /Q *.plg *.ncb *.opt *.aps *.suo *.sdf *.suo
del /Q /A:H *.suo

cd ..

rmdir /S /Q Debug Release
del /Q *.plg *.ncb *.opt *.aps *.suo *.sdf
del /Q /A:H *.suo

pause
