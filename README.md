# in2out
A windows desktop console application to receive, process and transmit data over TCP/IP

- The application starts from a command window
- The user specifies on command line the input port number to listen for incoming data
- The user specifies on command line IP address and port number to connect to for transmitting processed data
- Ascii data is received on the specified input port, one line at a time.
- When a line of data is received, it is passed to a processing function. ( This function will be easily modified by the client )
- The processed data returned from the processing function will transmitted to the specified IP and port
- The user can quit the application by typing EXIT

Sample command line

```
>in2out --input 5000 --output 127.0.0.1:50001
```
