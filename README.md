# Ghezavat_CSC357_Program_4

Hello,

You can start server with `./httpd PORT#`
You can start client with `./client unix# PORT#`

Get Request :

`GET PATH HTTP.1.0` like `GET /index.html HTTP/1.0`

HEAD Request: 

`HEAD PATH HTTP.1.0` like `HEAD /index.html HTTP/1.0`

GET responses and Error messages will print to an output file which will be creaetd within the `tests` folder which already has 2 example (one for an error, one for client ouptut)

ALL responses will also be printed to client terminal and will have appropiate headers.
