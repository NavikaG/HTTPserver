#include<err.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include <stdbool.h> 
#include <pthread.h>

int numPorts;
int healthCount=0;
int numReq;
int counter=0; 
int oneTimeUse=0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexTwo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexThree = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t wait = PTHREAD_COND_INITIALIZER;



struct tuple
{
    int port;
    int count;
    int fail;
    bool alive;
    bool valid;
} ;




struct tuple portfd[1000];





struct myNode
{

    int data;
    struct myNode *next;
};
typedef struct myNode node;

node *head = NULL;
node *tail = NULL;

void enqueue(int data)
{

    node *n = malloc(sizeof(node));
    n->next = NULL;
    n->data = data;

    if (head == NULL || tail == NULL)
    {
        head = n;
        tail = n;
    }
    else
    {
        tail->next = n;
        tail = n;
    }
}

int dequeue()
{

    int temp;
    node *hold;

    if (head == NULL || tail == NULL)
    {
        return  -1;
    }

    if (head == tail)
    {
        temp = head->data;
        hold = head;
        tail = NULL;
        head = NULL;
        free(hold);
        return temp;
    }
    else
    {
        temp = head->data;
        hold = head;
        head = head->next;
        free(hold);
        return temp;
    }
}















/*
 * client_connect takes a port number and establishes a connection as a client.
 * connectport: port number of server to connect to
 * returns: valid socket if successful, -1 otherwise
 */
int client_connect(uint16_t connectport) {
    int connfd;
    struct sockaddr_in servaddr;

    connfd=socket(AF_INET,SOCK_STREAM,0);
    if (connfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(connectport);

    /* For this assignment the IP address can be fixed */
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));

    if(connect(connfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
        return -1;
    return connfd;
}

/*
 * server_listen takes a port number and creates a socket to listen on 
 * that port.
 * port: the port number to receive connections
 * returns: valid socket if successful, -1 otherwise
 */
int server_listen(int port) {
    int listenfd;
    int enable = 1;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
        return -1;
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof servaddr) < 0)
        return -1;
    if (listen(listenfd, 500) < 0)
        return -1;
    return listenfd;
}

/*
 * bridge_connections send up to 100 bytes from fromfd to tofd
 * fromfd, tofd: valid sockets
 * returns: number of bytes sent, 0 if connection closed, -1 on error
 */
int bridge_connections(int fromfd, int tofd) {
    char recvline[4096];
    int n = recv(fromfd, recvline, 4096, 0);
    if (n < 0) {
        //printf("connection error receiving\n");
        return -1;
    } else if (n == 0) {
        //printf("receiving connection ended\n");
        return 0;
    }
    recvline[n] = '\0';
    //printf("%s", recvline);
    // sleep(1);
    n = send(tofd, recvline, n, 0);
    if (n < 0) {
        //printf("connection error sending\n");
        return -1;
    } else if (n == 0) {
        //printf("sending connection ended\n");
        return 0;
    }
    return n;
}

/*
 * bridge_loop forwards all messages between both sockets until the connection
 * is interrupted. It also prints a message if both channels are idle.
 * sockfd1, sockfd2: valid sockets
 */
void bridge_loop(int sockfd1, int sockfd2) {
    fd_set set;
    struct timeval timeout;

    int fromfd, tofd;
    while(1) {
        // printf("im stuck here\n");
        // set for select usage must be initialized before each select call
        // set manages which file descriptors are being watched
        FD_ZERO (&set);
        FD_SET (sockfd1, &set);
        FD_SET (sockfd2, &set);

        // same for timeout
        // max time waiting, 5 seconds, 0 microseconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // select return the number of file descriptors ready for reading in set
        switch (select(FD_SETSIZE, &set, NULL, NULL, &timeout)) {
            case -1:
                //printf("error during select, exiting\n");
                return;
            case 0:
                //printf("both channels are idle, waiting again\n");
                continue;
            default:
                if (FD_ISSET(sockfd1, &set)) {
                    fromfd = sockfd1;
                    tofd = sockfd2;
                } else if (FD_ISSET(sockfd2, &set)) {
                    fromfd = sockfd2;
                    tofd = sockfd1;
                } else {
                    //printf("this should be unreachable\n");
                    return;
                }
        }
        if (bridge_connections(fromfd, tofd) <= 0)
            // printf("negative\n");
                   
            return;
    }
}

void resetPort(struct tuple ports[]){
    for(int i=0; i<counter; i++){
        
            ports[i].alive=true;

    }
        


}

//only returns first port with smallest count for now
int pickPort(struct tuple ports[], int dead){


    for(int i=0; i<counter; i++){
        if(ports[i].port==dead){
            ports[i].alive=false;
        }

    }


    int smallest=100000;
    smallest=ports[0].count;
    for(int i=0; i<counter; i++){
        if(ports[i].count<smallest &&  ports[i].alive!=false && ports[i].valid!=false ){
            smallest=ports[i].count;
        }

    }

    int leastFail=1000000;

    for(int i=0; i<counter; i++){
        if(ports[i].count==smallest && ports[i].alive!=false && ports[i].valid!=false){
            if(ports[i].fail<leastFail){
                leastFail=ports[i].fail;
            }


            
        }
    }

    for(int i=0; i<counter; i++){
        if(ports[i].count==smallest && ports[i].alive!=false && ports[i].valid!=false){
            if(ports[i].fail==leastFail){
                return ports[i].port;
                ports[i].count++;
            }


            
        }
    }

    
    return -1;

}

void healthChecks(struct tuple ports[]){
    uint8_t buffer[4096];
    
    
     
    

    for( int i =0; i<counter; i++){
        if(ports[i].alive==true){
        //printf("port in healthcheck: %d\n", ports[i].port);

        // char accu[4096]="";
         
        memset(buffer, '\0', 4096);


        int fd = client_connect(ports[i].port); 

        char acc[4096]="";
    


        send(fd,"GET /healthcheck HTTP/1.1\r\n\r\n", strlen("GET /healthcheck HTTP/1.1\r\n\r\n"), 0);

        int check;


        struct timeval tv = {5, 0};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

        while (1){

            char buff[100]="";

            check = recv(fd, buff, 100, 0);
            // if(check==-1){
            //     //500
            // }
            strcat(acc, buff);

            //printf("check: %d\n", check);

            if(check==0){
                break;
            }

            if (check==-1){
                ports[i].valid=false;
                // printf("im in here -1");
                break;
                
            }

            



        }

        if(check!=-1){
        //printf("healthsss: %s\n", acc);

        
        char *sPtr;
        char *token = strtok_r(acc, "\n\r", &sPtr);
        if (token!=NULL){

        
            token = strtok_r(NULL, "\n\r", &sPtr);
            
            if (token!=NULL){
            token = strtok_r(NULL, "\n\r", &sPtr);

            if (token!=NULL){

        

            if(strcmp(token, "0")==0 || atoi(token)!=0){
                int numFail= atoi(token);


                ports[i].fail=numFail;
                ports[i].valid=true;

            }else{ports[i].valid=false;}

             }else{ports[i].valid=false;}

            }else{ports[i].valid=false;}


        }else{ports[i].valid=false;  }

        //printf("fail: %d\n", numFail);
        }

        
        // token = strtok_r(NULL, "\n\r", &sPtr);

        // printf("token: %s\n", token);


        //printf("valid: %d\n", portfd[i].valid);
        

        
        close(fd);


        // printf("port: %d", i);

    }
    //request healthcheck
    }

}
void handleReq(int acceptfd)
{

    resetPort(portfd);
    
    //possibley lock
    pthread_mutex_lock(&mutexTwo);
    int rightPort=pickPort(portfd,oneTimeUse);
    pthread_mutex_unlock(&mutexTwo);

    oneTimeUse=0;

    int connfd=client_connect(rightPort);



    while(connfd==-1){
    pthread_mutex_lock(&mutexTwo);
    rightPort=pickPort(portfd, rightPort);
    pthread_mutex_unlock(&mutexTwo);

    connfd=client_connect(rightPort);

    if(rightPort==-1){
        int check=write(acceptfd,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n", 58);
        check++;
        break;
        break;
    }

    }
    
    //printf("\n\nRIGHT PORT: %d\n\n", rightPort);


    if (rightPort!=-1){

     bridge_loop(acceptfd, connfd);

    }


    pthread_mutex_lock(&mutexThree);
    healthCount++;
    pthread_mutex_unlock(&mutexThree);
    

   

    close(acceptfd);

     if(healthCount%numReq==0){
        healthChecks(portfd);
        //printf("healthcount done: %d\n", healthCount);

    }
    

}




void *dispatcher()
{
    while (1)
    {
        //printf("im here now");

        int clientAddr;


        //if( (clientAddr = dequeue()) == -1)

        pthread_mutex_lock(&mutex);
        clientAddr = dequeue();
         if( clientAddr ==-1){
             pthread_cond_wait(&wait, &mutex);
            clientAddr= dequeue();
            
         }
        pthread_mutex_unlock(&mutex);





        if (clientAddr != -1)
        {

            handleReq(clientAddr);

            
        }
    }
}




int main(int argc,char **argv) {
    int listenfd; 
    int recPort=0;
    //acceptfd;
    uint16_t listenport;

    if (argc < 3) {
        //printf("missing arguments: usage %s port_to_connect port_to_listen", argv[0]);
        return 1;
    }

    int a;
    int numCon = 4;
    numReq = 5;
    
    

    int count=0;


    int ports[argc];

    

    for(int i=0; i<argc; i++){
        if(strlen(argv[i])==4 && atoi(argv[i])!=0){
                if ((strcmp(argv[i-1],"-N")!=0) && (strcmp(argv[i-1],"-R")!=0) ){

                    if (counter==0){
                       recPort= atoi(argv[i]);
                       counter++;
                    }
                    else{
                    ports[counter-1]=atoi(argv[i]);
                    counter=counter+1;
                    }
                }
            } 
    }
    
    counter--;
    //printf("recPort: %d\n", recPort);

    while ((a = getopt(argc, argv, "N:R:")) != -1)
    {
        switch (a)
        {
        case 'N':
            numCon = atoi(optarg);
            //printf("conections: %d\n", numCon);
            count = count + 1;
            break;

        case 'R':
            numReq = atoi(optarg);
            //printf("requests: %d\n", numReq);
            count = count + 1;
            break;

            // case '?':

        default:
            // port = (optarg);
            // printf("%s\n", optstring);
            //printf("here\n");
            break;
        }
    }


    numPorts = argc-2-count*2;
    // numPorts

    //printf("argc: %d\n", argc);

    //printf("NUM ports: %d\n", numPorts);

    

    //multithreading
    pthread_t threads[numCon];

    for (int i = 0; i < numCon; i++)
    {

        pthread_create(&threads[i], NULL, dispatcher, NULL);
    }





    // for (int i=argc-numPorts; i<argc; i++){
        
    //     printf("i:%s\n", argv[i]);
    //     ports[i]=atoi(argv[i]);


        
    // }
    //printf("here 1"); 
   
       

    // for(int i=0; i<4; i++)
    // {
    //      printf("ports:%d\n", ports[i]);
    // }
    
    

    // Remember to validate return values
    // You can fail tests for not validating

    listenport = recPort;
    if ((listenfd = server_listen(listenport)) < 0)
        err(1, "failed listening");

    

    

   

    //printf("here");

     
    
    for( int i =0; i<counter; i++){

        //printf("port: %d\n", i);

    //    int fd = client_connect(ports[i]); 
       portfd[i].port = ports[i];
    //    portfd[i].fd = fd;

       portfd[i].count = 0;

        portfd[i].alive = true;
        portfd[i].valid = true;




    }

    for( int i =0; i<counter; i++){

       
       //printf("\nport: %d\n", portfd[i].port);
    //    printf("fd: %d\n", portfd[i].fd);
       //printf("count: %d\n", portfd[i].count);




    }



    //printf("here 2 \n");

    
    healthChecks(portfd);

    while (true)
    {

    // connectport = pickPort();
   
    // connfd = client_connect(connectport);
    int acceptfd;
    
    if ((acceptfd = accept(listenfd, NULL, NULL)) < 0)
        err(1, "failed accepting");
    
    //printf("healthcount: %d\n", healthCount);



    //multithreading
    pthread_mutex_lock(&mutex);
    
    enqueue(acceptfd);
    pthread_cond_signal(&wait);
    pthread_mutex_unlock(&mutex);





    // This is a sample on how to bridge connections.
    // Modify as needed.

    // int connfd = pickPort()
    
    }
}
