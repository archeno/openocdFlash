openocd -f "target/02stlink.cfg" -c "program flash_files/%1 %2" -c "reset" -c "shutdown"