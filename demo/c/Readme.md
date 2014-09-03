## Design Notes

   This C program demo is very <strong>simple</strong> and <strong>stupid</strong>!

   The server will serve every client by a independent thread. 
   
   It striticly follow the method which synchronized request and response and based on the order. So it only uses the Blocking I/O model and send the data to client line by line.  

   When the client connect to the sever, client needs send which line he needs, such as: 2, which means I need the second line of the data. Client's threads will be synchronized to make sure the data is requested with the order.

   There are many spaces can be optimized. 
   
   Good Luck!

## Usage
 
  * Build: `make`
  
    Using `make` to build `server` and `client`. 

  * Server: `./server <port> <data file>`

    `server` is the program prepare the data.
    
    It needs those arguments: 
    
    * `port` : the TCP port to listen.
    * `data file` : the file need to sync to client line by line.
 
    Please wait server output "`Load the data successfully`" then you can use client get the data.
 
  * Client: `./client <server ip> <server port> [thread num] > <output file>`

    `client` is the program takes the data from server, and rediect the output into a file. 
    
    It needs those arguments:

    * `server ip` : server's IP address (do not support the host name).
    * `server port` : server's TCP port.
    * `thread num` : how many threads do we need to take the data? (by default: 128)
    * `output file` : the outpu file name

## Test

   for 1GB data with 56 client threads, it needs 35s, and the bandwidth usage is around 165Mb.