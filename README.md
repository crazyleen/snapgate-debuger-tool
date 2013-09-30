# snapgate-debuger

this programs is for eclipse to communicate with snapgate

* PC: snapgate-debuger-client
* ARM: snapgate-debuger-server


## usage:
	
list usb device:
	
	./snapgate-debuger-client <ip address> list
		
start gdb proxy:
	
	./snapgate-debuger-client <ip address> <usb serial> <port>
		
download program and start gdb proxy:
	
	./snapgate-debuger-client <ip address> <usb serial> <port> <filename>
	
download program:
	
	./snapgate-debuger-client <ip address> <usb serial> <port> <filename> prog
		
