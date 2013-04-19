#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define  PORT_NUM 6002


void oracle(int clientSocket, int myListenSocket, char*argv[]);
void guesser(int ClientSocket, char *argv[]);
int checkWin ( char secret[], int cGuesses[]);
int checkGuess(char secret[], int cGuesses[], char buffer[]);
void returnString(char secret[], int cGuesses[], char rTurn[]);
void initCGuesses(int guesses[]);
void printReturnString(char buffer[]);
void printHangman(int  bg);


int main(int argc, char *argv[]){
  if( argc == 1 ){
	// become the server
		// create neccesary variables 
		int  clientSocket;
  		struct sockaddr_in  myAddr, clientAddr;
  		int i, addrSize, bytesRcv;
  		char buffer[300];				
		// create socket
		int myListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(myListenSocket < 0){
			printf("error creating socket");
			exit(-1);		
		}
		//setup address	
		memset(&myAddr, 0, sizeof(myAddr));
  		myAddr.sin_family = AF_INET;
  		myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  		myAddr.sin_port = htons((unsigned short) PORT_NUM);
		// bind the socket
 	        i = bind(myListenSocket,
           	(struct sockaddr *) &myAddr,
           	sizeof(myAddr));
 		if (i < 0) {
    			printf("error could not bind socket\n");
    			exit(-1);
  		}
		// listen for a request
		  i = listen(myListenSocket, 5);
 		  if (i < 0) {
    		  	printf("error could not listen\n");
    		  	exit(-1);
  		  }
		// accept the request
		printf("waiting for client to connect...\n");
		addrSize = sizeof(clientAddr);
  		clientSocket = accept(myListenSocket,
                               (struct sockaddr *) &clientAddr,
                               &addrSize);
  		if (clientSocket < 0) {
    			printf("error couldn't accept the connection\n");
    			exit(-1);
  		}
		// call oracle and start game
		oracle(clientSocket, myListenSocket, argv);

	// become the client
	}else if (argc == 2){
		printf("setting up client ... \n");
		int mySocket;
		struct sockaddr_in  addr;
  		int i;

		/* create socket */
  		mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  		if (mySocket < 0) {
    		printf("error couldnt not create the socket\n");
    		exit(-1);
 		 }		
		
		/* setup address */
 		memset(&addr, 0, sizeof(addr));
  		addr.sin_family = AF_INET;
 		addr.sin_addr.s_addr = inet_addr(argv[1]);
  		addr.sin_port = htons((unsigned short) PORT_NUM);
		
		// connect to server
		printf("attempting to connect...\n"); 
		i = connect(mySocket,
                (struct sockaddr *) &addr,
                sizeof(addr));
  		if (i<0) {
    			printf("client could not connect!\n");
    			exit(-1);
 		 }
		guesser(mySocket,argv);	
	}
	




	return 0; 
}
void oracle(int clientSocket, int myListenSocket, char *argv[]){
	printf("you are now the oracle\n");
	printf("Enter a word for your opponnent to guess \n");
	char secret[30];
	int  cGuesses[30];
	char bGuesses[36];
	int  blength= 0; 
	initCGuesses(cGuesses);
	gets(secret);
	int badGuessesLeft = 6;
	int t = 1;
	char response[1];
	while(t==1){
		char buffer[30];
		printf("asking the other player if they would like to guess a word...\n");
		int i = recv(clientSocket,buffer,sizeof(char),0);
		if(buffer[0] == 'y'){
			 i = recv(clientSocket,buffer,sizeof(buffer),0);
			 buffer[i] = 0; 
		}else{
			i = recv(clientSocket,buffer,sizeof(char),0);
			buffer[i] = 0;
			bGuesses[blength] = buffer[0];
			blength+=1;		

		}
		badGuessesLeft += checkGuess(secret, cGuesses,buffer);
		if(badGuessesLeft  == 0){
			printf("you won the game the other player has guessed incorrectly 6 times\n");
			// tell the client they lost 
			buffer[0] = 'L'; 
			send(clientSocket,buffer,sizeof(char),0); 			
			printf("would you like to play again? ('y' or 'n')\n");
			scanf("%s", response);
			if(response[0] == 'n'){
				// tell the client your not playing again
				buffer[0] = response[0];
				send(clientSocket,buffer,sizeof(char),0);
				printf("GoodBye!\n");
				// do all neccesary clean up
				close(clientSocket); 
				close(myListenSocket);
				exit(0);	

			}else{
			 // tell the client your playing again
			 	buffer[0] = response[0];
				send(clientSocket,buffer,sizeof(char),0);				 
			 // restart as server again
			close(clientSocket);
			close(myListenSocket); 
			execvp("./a.out",argv);			
			}
		}

		if(checkWin(secret, cGuesses) == 0){
			printf("you lost the game the other player has guessed your word\n");
			// tell the client they won
			buffer[0] = 'W' ;
                        i= send(clientSocket,buffer,sizeof(char),0);
			// ask the client if they want to play again
			printf("Asking the other player if they want to play again\n");
			i = recv(clientSocket,buffer,sizeof(char),0);
			buffer[i] = 0;
			if(buffer[0] == 'y'){
			// ask the user of the ip to connect to
				printf("you are about to restart as a client,  please enter an ip adress to connect to\n");
				char ip[30];
				scanf("%30s", ip);
				char *argv[2];
				argv[0] = "./a.out";
				argv[1] = ip;
				close(clientSocket);
				close(myListenSocket);
				execvp("./a.out",argv);
			}
			if(buffer[0] == 'n'){
			 // start again as server
			 printf("starting a new game\n");
			 close(clientSocket);
			 close(myListenSocket);
			 execvp("./a.out",argv);			 
			}
		}
		// tell the client the game is not over and send the return string
		buffer[0] = 'N';
		buffer[1] = (char)badGuessesLeft;
		i = send(clientSocket,buffer,2*sizeof(char),0);
		// send the return string
		char rTurn[30] = " ";
		returnString(secret,cGuesses,rTurn);
		send(clientSocket,rTurn,sizeof(rTurn),0);
		printReturnString(rTurn);
		printHangman(badGuessesLeft);		
	}

}
int checkWin ( char secret[], int cGuesses[]){
		int i;
		for(i = 0; i < strlen(secret); i++){
			//	printf("cGuesses[%d] = %d \n",i,cGuesses[i] );
			if(cGuesses[i] == 0 ){
//				printf("should return -1 \n"); 
				return -1;
			}
		}
	return 0;
}
int checkGuess(char secret[], int cGuesses[], char buffer[]){
	int i;
	int truth = -1;
	if(strlen(buffer) == 1){
	//	printf("letter was guessed \n"); 
		for(i = 0; i< strlen(secret); i++){
			//printf(" cguesses[%d] = %d \n", i, cGuesses[i]);
			if(secret[i] == buffer[0]){
				cGuesses[i] = 1;
				truth = 0;
				}
		}
	}else if (strlen(buffer) == strlen(secret) ){
		int count = 0;
		for(i=0; i < strlen(secret); i ++){
			if(secret[i] == buffer[i])
				count+=1;
		//	printf("count is %d", count);
		}
		if(count == strlen(secret)){
 			truth = 0;
			for(i=0; i< strlen(secret); i++)
				cGuesses[i] = 1;
		}
	}
	if( truth == 0){
		printf("other player guessed correctly\n");
	}else{
		printf("other player guessed incorrectly\n");
	}
 return truth;	
}
void returnString(char secret[], int cGuesses[], char rTurn[]){
	char rString[30];
	int i;
	for(i =0; i < strlen(secret); i ++){
		if(cGuesses[i] == 1) rString[i] = secret[i];
		else{
			rString[i] = '*';
		}
	}
	 rString[i+1] = 0;	
	 strcpy(rTurn,rString);
}
void printReturnString(char buffer[]){
	printf("letters guessed so far  : ");
	printf("%s \n", buffer);
}

void printHangman(int  bg){
	if(bg < 6)printf(" ( ) \n");
	if(bg < 5)printf("/");
	if(bg < 4)printf("|");
	if(bg < 3)printf("\\  \n");
	if(bg < 2)printf("/");
	if(bg < 1)printf("\\  \n");
	printf("\n");
	printf("%d guesses left \n", bg);

}
void initCGuesses(int guesses[]){
	int i;
	for(i = 0 ; i< 30; i++){
		guesses[i] = 0;	
	}
	
}

void guesser(int ClientSocket, char *argv[]){
