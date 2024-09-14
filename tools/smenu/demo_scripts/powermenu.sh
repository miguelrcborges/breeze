#!/bin/sh

OPERATION="$(echo -e 'poweroff\nreboot\ncancel' | smenu)"

case OPERATION in
	"poweroff")
		cmd.exe /C "shutdown /s /t 0"
		;;
	"reboot")
		cmd.exe /C "shutdown /r /t 0"
		;;
esac
