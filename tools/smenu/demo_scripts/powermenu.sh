#!/bin/sh

OPERATION="$(echo -e 'poweroff\nreboot\nlog off\ncancel' | smenu)"

case $OPERATION in
	"poweroff")
		cmd.exe //c "shutdown /s /t 0"
		;;
	"reboot")
		cmd.exe //c "shutdown /r /t 0"
		;;
	"log off")
		cmd.exe //c "shutdown /l"
		;;
esac
