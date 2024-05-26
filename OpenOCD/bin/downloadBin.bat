openocd -f ./cmsis-dap.cfg  -c  "program ../../app.bin 0x08000000 verify reset  exit"
pause 