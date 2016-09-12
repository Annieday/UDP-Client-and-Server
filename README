Name:               Anni Dai
Student ID number:  1264320
Unix id:            adai1
Lecture section:    B1
Instructor's name:  Mohammad Bhuiyan
Lab section:        H02

This tar contains file_server and file_client.

How to compile:
	By type "make" or "make file_server", you will compile file_server.
	By type "make file_client", you will compile file_client.
	By type "make all", you will compile both server and client.

How to run file_server:
The server is started with three command line arguments:
1)the UDP port it will listen for.
2)a directory in which the server serves up documents from 
3)a log file in which the server logs all transactions with all clients
e.g:
./file_server 9090 /some/where/documents /some/where/logfile

The server accepts a request from a client that only sends the file name to be sent. The server
splits the intended file into chunks of 1KB and sends each chunk as a separate message to
the client. If the file has a size that is a multiple 1KB, the server will send an additional
message containing only “$” after sending all of the chunks.

The server logs all transactions in the log file, one line for each request. Each line has the
following format:
<client IP> <client port> <name of the file to be sent> <time when request
received> <time when file transmission completed>
Or
<client IP> <client port> <name of the file to be sent> <time when request
received> <file not found>
Or
<client IP> <client port> <name of the file to be sent> <time when request
received> <transmission not completed>

sample log:
129.128.41.18 52367 RE Fri 20 Mar 2015 19:13:29 MDT file not found
129.128.41.18 48461 README Fri 20 Mar 2015 19:13:44 MDT Fri 20 Mar 2015 19:13:44 MDT

How to run file_client:
The client is started with three command line arguments:
1)the server IP address
2)the UDP port
3)a transfer request of filename
e.g:
./file_client 192.168.10.1 9090 filename

The client accepts chunks of messages from the server. If a chunk contains less than 1KB of
data or only the message “$”, the desired file transmission is completed. In the latter case,
the client should ignore the message “$”. We assume that if a file has just one more byte
beyond the multiple of 1KB, the last byte is not a ‘$’. For an active file transfer, if 5 seconds
have been passed since the last chunk received, the client assumes that the transmission
had been aborted, and the client prints the error message on the screen.

** The received file content will write to the screen

