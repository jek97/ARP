// server
// declare some variables for the socket:
    int sockfd, newsockfd, clilen, n; // declare the socket file descriptors, the size of the address of the client and the character readed and writed on the socket
    int portno = 50000; // Declare the port number (to decide)
    struct sockaddr_in serv_addr, cli_addr; // structure to store the server and client internet address


    char out_buf[5]; // declare the buffer in output
    int w_pA; // declare the returned valeu of the write function on the socket

    // create the socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // open the socket
    if (sockfd < 0) {
        perror("error opening the socket from processA"); // checking errors
        logger(log_pn_processA, "e0001"); // write a log message
    }
    else {
        logger(log_pn_processA, "0001"); // write a log message
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // set all the values of the server address buffer equal to zero
    serv_addr.sin_family = AF_INET; // set the address family
    serv_addr.sin_port = htons(portno); // set the port number
    serv_addr.sin_addr.s_addr = INADDR_ANY; // set the IP address of the host
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // binding the socket to the port
        perror("error binding the socket from processA"); // checking errors
        logger(log_pn_processA, "e0010"); // write a log message
    }
    else {
        logger(log_pn_processA, "0010"); // write a log message
    }

    listen(sockfd, 5); // listen for connections (max 5)
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // enstablish the connection with the client

    if (newsockfd < 0) {
        perror("error waiting from connections in processA"); // checking errors
        logger(log_pn_processA, "e0011"); // write a log message
    }
    else {
        logger(log_pn_processA, "0011"); // write a log message
    }




// client
// declare some variables for the socket:
    int sockfd, newsockfd, clilen, n; // declare the socket file descriptors, the size of the address of the client and the character readed and writed on the socket
    int portno = 50000; // Declare the port number (to decide)
    struct sockaddr_in serv_addr; // structure to store the server internet address
    const char * server_nam = "Lazymachine"; // name of the server (to decide)
    struct hostent * server; // pointer to a struct containing the alias of the server

    int r_pB; // declare the returned variable of the read function on the socket
    char in_buf[5]; // declare the buffer in input
    int in = 0; // initialize the variable where i will store the valeu received


    // create the socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // open the socket
    if (sockfd < 0) {
        perror("error opening the socket from processB"); // checking errors
        logger(log_pn_processA, "e011101"); // write a log message
    }
    else {
        logger(log_pn_processA, "011101"); // write a log message
    }

    server = gethostbyname(server_nam); // fill the struct with the server data
    if (server == NULL) {
        perror("error obtaining the host data from processB"); // checking errors
        logger(log_pn_processA, "e011110"); // write a log message
    }
    else {
        logger(log_pn_processA, "011110"); // write a log message
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // set all the values of the server address buffer equal to zero
    serv_addr.sin_family = AF_INET; // set the address family
    bcopy((char *) server -> h_addr_list, (char *) &serv_addr.sin_addr.s_addr, server -> h_length); // set the fields in serv_addr
    serv_addr.sin_port = portno; // set the fields in serv_addr

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // connecting to server
        perror("error connecting to server from processB"); // checking errors
        logger(log_pn_processA, "e011111"); // write a log message
    }
    else {
        logger(log_pn_processA, "011111"); // write a log message
    }
    
