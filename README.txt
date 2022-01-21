==============================================================================

		  EE450 Socket Programming Project
		  
==============================================================================

I. What I have done

   In this project, I have succefully implemented all the required 
   functionalities of three phases with two client files and four 
   server files.  I did all of my programming and debug testing work 
   on USC Student VM and it worked well as expected.
   
==============================================================================

II. File Structure

   1.clientA.c
	   Code for clientA to communicate with Central server via TCP. Send 
	   hostname A to the central server and receive compatibility info
	   from the central server.

   2.clientB.c
	   Code for clientB to communicate with Central server via TCP. Send 
	   hostname B to the central server and receive compatibility info
	   from the central server.

   3.central.c
	   Code for serverC to communicate with two clients via TCP and three 
	   backend-servers via UDP. Get hostnames from clients, send request to 
	   each backend-server in order, and send the results back to clients.

   4.serverT.c
	   Code for serverT to communicate with central server via UDP. Receive 
	   the request from the central server, construct topology from edgelist
	   file and send it back to the central server.

   5.serverS.c
	   Code for serverS to communicate with central server via UDP. Receive 
	   the request from the central server, extract scores of each node 
	   from score file, and send it back to the central server.

   6.serverP.c
	   Code for serverS to communicate with central server via UDP. Receive 
	   topology and scores to calculate the final result and send it back 
	   to the central server.

==============================================================================

III. Format of message

   Example Output to Illustrate Output: User Inputs are Rachael and Oliver

   1.clientA Terminal
	   The client is up and running
	   The client sent <Rachael> to the Central Server
	   Found compatibility for <Rachael> and <Oliver>:
	   <Rachael>--<Oliver>
	   Matching Gap:<0.37>

   2.clientB Terminal
	   The client is up and running
	   The client sent <Oliver> to the Central Server
	   Found compatibility for <Oliver> and <Rachael>:
	   <Oliver>--<Rachael>
	   Matching Gap:<0.37>

   3.Central Server Terminal
	   The Central Server is up and running
	   The Central Server received input = < Rachael > from the client A 
	   using TCP over port<25047>
 	   The Central Server received input = < Oliver > from the client B 
 	   using TCP over port<26047>       
	   The Central Server sent a request to Backend Server T
	   The Central Server received information from Backend-server T using 
	   UDP over port 21047
	   The Central Server sent a request to Backend Server S
	   The Central Server received information from Backend-server S using 
	   UDP over port 22047
	   The Central Server sent a processing request to Backend-Server P
	   The Central server received the results from backend server P
	   The Central server sent the results to client<B>
	   The Central server sent the results to client<A>

   4.Backend-Server T Terminal
	   The ServerT is up and running using UDP on port 21047
	   The serverT received a request from Central to get the topology
	   The serverT finished sending the topology to central

   5.Backend-Server S Terminal
	   The ServerS is up and running using UDP on port 22047
	   The serverS received a request from Central to get the scores
	   The serverS finished sending the scores to Central

   6.Backend-Server P Terminal
	   The server P is up and running using UDP on port 23047
	   The serverP received the topology and score information
	   The serverP finished sending the results to the Central

==============================================================================

IV. Idiosyncrasy in Project
        
   To handle the variable strings in C, there are lot of buffer with 
   large size. The max length of buffers are set to 4000. If a single 
   message or file exceeds this size, the program will crash.

==============================================================================

V. Reused Code
        
   1.The implementation of TCP and UDP refers to Beej's Guide to Network 
   Programming. Each step is preceded by a comment.

   2. The implementation of Dijkstra Algorithm is based on code from CSDN 
   with some modifications.
	


	






	
