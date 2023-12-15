# Ghezavat_CSC357_Program_4

Hello,

You can start the server with `./httpd PORT#`
You can start the client with `./client unix# PORT#`

Get Request :

`GET PATH HTTP/1.0` like `GET /index.html HTTP/1.0`

HEAD Request: 

`HEAD PATH HTTP/1.0` like `HEAD /index.html HTTP/1.0`

GET responses and Error messages will print to an output file which will be created within the `tests` folder which already has 2 examples (one for an error, one for a successful client request)
The errors will have `*error*` at the beginning of their output file name and the successful client requests will just have `client: *pid#*` at the beginning of their output file name

ALL responses will also be printed to the client terminal and have appropriate headers. (GET headers will only print in client terminal NOT the output file, as advised in Office Hours)

Client should terminate after a response is generated.
