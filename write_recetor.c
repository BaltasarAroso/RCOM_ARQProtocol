/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TX 1
#define RX 0

/*custom defines*/
#define MAX_SIZE 4096

/*Flags*/
#define FR_F        0x7e    //0111 1110
#define FR_F_AUX    0x7d5e  //0111 1101 0101 1110

#define ESC         0x7d    //0111 1101
#define ESC_AUX     0x7d5d  //0111 1101 0101 1101 

#define FR_A_RX     0x03    //0000 0011 
#define FR_A_TX     0x01    //0000 0001

#define FR_C_SET    0x03    //0000 0011
#define FR_C_DISC   0x0B    //0000 1011
#define FR_C_UA     0x07    //0000 0111
#define FR_C_RR0    0x05    //0000 0101 -> readNumber = 1 -> S = 1 -> R = 0 -> Pronto para receber a trama 0
#define FR_C_RR1    0x85    //1000 0101 -> readNumber = 0 -> S = 0 -> R = 1 -> Pronto para receber a trama 1
#define FR_C_REJ0   0x01    //0000 0001 -> readNumber = 1 -> S = 1 -> R = 0 -> Rejeita o envio da trama 0
#define FR_C_REJ1   0x81    //1000 0001 -> readNumber = 0 -> S = 0 -> R = 1 -> Rejeita o envio da trama 1

#define FR_C_SEND0  0x00    //0000 0000
#define FR_C_SEND1  0x40    //0100 0000

/* GLOBAL VAR */
int readNumber = 0; //Numero de trama a ler
int sendNumber = 0; //Numero de trama a escrever 

int llopen(int port, int MODE);
int llwrite(int port, char *buffer, int length);
int llread(int port, char *buffer);
int llclose(int port);

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[MAX_SIZE];
    unsigned char teste[MAX_SIZE];
    int i, sum = 0, speed = 0;
    int retval = 0;

    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused (estava a 0)*/
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received (estava a 5)*/



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


	/*CHAMAR LLOPEN() */
	fprintf(stderr, "SHALL WE BEGIN?... RX / TX?\n\n");
	if(fgets(buf, sizeof(buf), stdin) == 0){
		perror("fgets");
		exit(-1);
	}
	if(strncmp(buf,"TX", 2) == 0){
		if (llopen(fd, TX) < 0){
			perror ("llopen()");
			exit(-1);
		}
		if (llwrite(fd,"Baltas- ar\n",10) < 0){
			perror ("llwrite()");
			exit(-1);
		}
	} else if(strncmp(buf, "RX", 2) == 0) {
		if (llopen(fd, RX) < 0){
			perror ("llopen()");
			exit(-1);
		}
		if (llread(fd,teste) < 0){
			perror ("llread()");
			exit(-1);
		}
	} else {
		perror("Bad input: Use 'RX' or 'TX' as an argument");
		exit(-1);
	}

	fprintf(stderr, "llread(): %s\n", teste);


	/*Camada de aplicacao*/
	//START
	/*int i = 0, j = 0, tam_V1 = 0; tam_V2 = 0;
	if (frame_read[4]==0x02){
		buffer[0] = frame_read[4];
		buffer[1] = frame_read[5];
		buffer[2] = frame_read[6];
		tam_V1 = (int)strtol(buffer[2], NULL, 0);
		for(i=3; i<tam_V1+3; i++){
			buffer[i] = frame_read[i+4];
		}
		buffer[i] = frame_read[i+4]; i++;
		buffer[i] = frame_read[i+4]; i++;
		tam_V2 = (int)strtol(buffer[i], NULL, 0);
		j=i;
		for(i=j; i<tam_V2+j; i++){
			buffer[i] = frame_read[i+4];
		}
	}*/
	//INFO
	/*if(frame_read[4]==0x00){

	}*/
	//END
	/*if(frame_read[4]==0x03){

	}*/
 

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guião 
  */

	sleep(1); //para o set de default não alterar durante a escrita
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {  //volta a pôr a configuração original
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}

int llopen(int port, int MODE){

	int countf = 0, res = 0, i = 0, state = 0, b = 0, bad = 0;
	unsigned char SET[5], UA[5];
	unsigned char frame_read[MAX_SIZE];
	time_t oldtime;

	/* SET */
	SET[0] = FR_F;
	SET[1] = FR_A_RX;
	SET[2] = FR_C_SET;
	SET[3] = SET[1]^SET[2];
	SET[4] = FR_F;

	/* UA */
	UA[0] = FR_F;
	UA[1] = FR_A_RX;
	UA[2] = FR_C_UA;
	UA[3] = UA[1]^UA[2];
	UA[4] = FR_F;
	
init:
	if(MODE == TX) {
		//ESCREVE SET
		res = write(port, SET, 5);
		if((oldtime = time(NULL)) < 0){
			perror("timer");
		}

		fprintf(stderr, "\nSET frame sent: |%x|%x|%x|%x|%x|;\t bytes sent: %d\n\n", SET[0], SET[1], SET[2], SET[3], SET[4], res);

		//ESPERA UA
		while(state == 0){
			res = read(port, &frame_read[b], 1);
			if(res == 0) {
				if(time(NULL) - oldtime >= 3){
					fprintf(stderr, "Warning: connection timed out; Re-sending message...\n");
					bad++;
					if(bad >= 3){
						state = 1;
						break;
					} else {
						goto init;
					}
					oldtime = time(NULL);
				}
				continue;
			}
			if(frame_read[b] == FR_F) {
				countf++;
			}
			if(countf == 2){
				countf = 0;
				if(frame_read[b] == frame_read[b-1]){
					b = 0;
					frame_read[b] = FR_F;
				} else {
					state = 1;
					break;
				}
			}
			b++;
		}

		//VER SE RECEBEU UA
		if(strncmp(frame_read, UA, 5) != 0) {
			fprintf(stderr, "BAD BAD WOLF TX\n");
			return -1;
		} else {
			fprintf(stderr, "GOOD DOGGY DOG TX\n");
		}

	} else if(MODE == RX) {

		if((oldtime = time(NULL)) < 0){
			perror("timer");
		}
		//LEITURA DE SET
		while(state == 0){
			res = read(port, &frame_read[b], 1);
			if(res == 0) {
				if(time(NULL) - oldtime >= 18){
					fprintf(stderr, "Warning: timed-out; no request received...\n");
					return -1;
				} else {
					continue;
				}
			}
			if(frame_read[b] == FR_F) {
				countf++;
			}
			if(countf == 2){
				countf = 0;
				if(frame_read[b] == frame_read[b-1]){
					b = 0;
					frame_read[b] = FR_F;
				} else {
					state = 1;
					break;
				}
			}
			b++;
		}

		/*VER SE RECEBEU SET CORRETAMENTE*/
		if(strncmp(frame_read, SET, 5) != 0) {
			fprintf(stderr, "BAD BAD WOLF RX\n");
			if(bad < 3) {
				bad++;
				goto init;
			} else {
				return -1;
			}
		} else {
			fprintf(stderr, "GOOD DOGGY DOG RX, sending UA...\n");

			//ENVIAR UA
			res = write(port, UA, 5);
			fprintf(stderr, "\nUA frame sent: |%x|%x|%x|%x|%x|;\t bytes sent: %d\n\n", UA[0], UA[1], UA[2], UA[3], UA[4], res);
		}

	} else {
		return -1;
	}

	
	return 0;
}

int llwrite(int port, char* buffer, int length) {

    int countf = 0, i = 0, b = 0, done = 0, sent = 0, res = 0, frame_len = 0, state = 0, bad = 0;
	unsigned char RR[5], REJ[5];
	unsigned char frame[MAX_SIZE];
	unsigned char answer[MAX_SIZE];
	unsigned char Bcc2 = 0x00;
	time_t oldtime;

	//RR
	RR[0] = FR_F;
	RR[1] = FR_A_RX;
	if(sendNumber == 0) {
		RR[2] = FR_C_RR1;
	} else if(sendNumber == 1) {
		RR[2] = FR_C_RR0;
	}
	RR[3] = RR[1]^RR[2];
	RR[4] = FR_F;

	//REJ
	REJ[0] = FR_F;
	REJ[1] = FR_A_RX;
	if(sendNumber == 0) {  //R = 1 - S
		REJ[2] = FR_C_REJ1;
	} else if(sendNumber == 1) {
		REJ[2] = FR_C_REJ0;
	}
	REJ[3] = REJ[1]^REJ[2];
	REJ[4] = FR_F;

	//construir trama
	frame[b++] = FR_F;  //Flag inicial
	frame[b++] = FR_A_RX;  //campo A
	if(sendNumber == 0) {   //campo C
		frame[b++] = FR_C_SEND0;
	} else if(sendNumber == 1) {
		frame[b++] = FR_C_SEND1;
	}
	frame[b++] = frame[1]^frame[2];  //Bcc1
	for( i=0; i<length; i++){    //Byte stuffing dos dados
		Bcc2 = Bcc2^buffer[i];   //Bcc2 feito com XOR dos dados originais
		if(buffer[i] == 0x7E) {
			frame[b++] = 0x7D;
			frame[b++] = 0x5E;
		} else if(buffer[i] == 0x7D) {
			frame[b++] = 0x7D;
			frame[b++] = 0x5D;
		} else {
			frame[b++] = buffer[i];
		}
	}

	if(Bcc2 == 0x7E) {  //Byte stuffing do Bcc2
		frame[b++] = 0x7D;
		frame[b++] = 0x5E;
	} else if(Bcc2 == 0x7D) {
		frame[b++] = 0x7D;
		frame[b++] = 0x5D;
	} else {
		frame[b++] = Bcc2;  //Bcc2
	}

	frame[b] = FR_F;  //Flag final
	frame_len = b+1;  //tamanho da frame = pos final + 1
	b = 0;

	//enviar trama
	while(!done) {
		switch (state) {
			case 0:  //escrita da trama na porta
				oldtime = time(NULL);
				sent = write(port, frame, frame_len);
				fprintf(stderr, "\nframe sent: ");
				for(i=0; i<frame_len; i++) {
					fprintf(stderr, "|%x|",frame[i]);
				}
				fprintf(stderr, " %d \n", frame_len);
				state = 1;
				break;

			case 1:  //espera ack ou nack
				while(state == 1){
					res = read(port, &answer[b], 1);
					if(res == 0) {
						if(time(NULL) - oldtime >= 3){ //time out
							fprintf(stderr, "Warning: connection timed out; Re-sending message...\n");
							bad++;
							if(bad >= 3){
								fprintf(stderr, "Error: Connection timed out.\n");
								return -1;  //ao fim de 3 time outs não tenta mais
							} else {
								state = 0;  //reenvia frame
							}
							oldtime = time(NULL);
						}
						continue;
					}
					if(answer[b] == FR_F) {
						countf++;
					}
					if(countf == 2) {
						countf = 0;
						if(answer[b] == answer[b-1]){  //2 flags seguidas
							b = 0;  //descarta tudo
							answer[b] = FR_F;  //reinicia answer
						} else {
							state = 2;
							break;
						}
					}
					b++;
				}
				break;

			case 2:  //analisar resposta
				// Distinguir ACK ou NACK
				if(strncmp(answer, RR, 5) != 0) {
					sendNumber = 1 - sendNumber;
					break;
				} else if (strncmp(answer, REJ, 5) != 0){
					state = 0;
				}

			default: break;
		}
	}

	//retornar caracteres escritos
	return sent;

}

int llread(int port, char *buffer){

	int state = 0, res = 0, bad = 0, countf = 0, b = 0, wrong = 0, len = 0, i = 0, j = 0;
	unsigned char RR[5], REJ[5];
	unsigned char frame_read[MAX_SIZE];
	unsigned char BCC2 = 0x00;
	time_t oldtime;

	/* RR */
	RR[0] = FR_F;
	RR[1] = FR_A_RX;
	RR[2] = FR_C_RR0;   //standard
	RR[3] = RR[1]^RR[2];
	RR[4] = FR_F;

	/* REJ */
	REJ[0] = FR_F;
	REJ[1] = FR_A_RX;
	REJ[2] = FR_C_REJ0; //standard
	REJ[3] = REJ[1]^REJ[2];
	REJ[4] = FR_F;	

	//ler a trama	
init:
	if((oldtime = time(NULL)) < 0){
		perror("timer");
	}

	while(state == 0){
		res = read(port, &frame_read[b], 1);
		if(res == 0) {
			/*if(time(NULL) - oldtime >= 3){
				fprintf(stderr, "Warning: connection timed out; Re-sending message...\n");
				bad++;
				if(bad >= 3){
					state = 1;
					fprintf(stderr, "Error: connection time-out; Try again...\n");
					return -1;
				} else {
					goto init;
				}
				oldtime = time(NULL);
			}*/
			continue;
		}
		if(frame_read[b] == FR_F) {
			countf++;
		}
		if(countf == 2){
			countf = 0;
			if(frame_read[b] == frame_read[b-1]){
				b = 0;
				frame_read[b] = FR_F;
			} else {
				state = 1;
				break;
			}
		}
		b++;
	}
	b++;

	len=b;
	//no final do ciclo frame_read tem o tamanho de len bytes	

	//decifrar os cabecalhos
	if (frame_read[1]^frame_read[2]!=frame_read[3]){
		wrong = 1;
	}
	if (wrong == 1){
		goto end;
	}
	
	//primeiro mecanismo de destuffing do pacote (D1...Dn) e do BBC2 (1 byte)
	for (i=4; i<len-1; i++){
		if(frame_read[i]==0x7D && frame_read[i+1]==0x5E){
			frame_read[i] = 0x7E;
			for (j=i+1; j<len-1; j++){
				frame_read[j]=frame_read[j+1];
			}
		}
		if(frame_read[i]==0x7D && frame_read[i+1]==0x5E){
			frame_read[i] = 0x7D;
			for (j=i+1; j<len-1; j++){
				frame_read[j]=frame_read[j+1];
			}
		}
	}

	//comparacao de BCC2 com o XOR do pacote (D1^D2^...^Dn)
	for (i=4; i<len-2; i++){
		BCC2 ^= frame_read[i];
	}
	if (BCC2 != frame_read[len-2]){
		wrong = 1;
	}

	//enviar RR ou REJ
	if (wrong == 0){
		if(frame_read[2]==0x00){
			readNumber = 0;
			RR[2] = FR_C_RR0;
		} else if (frame_read[2]==0x40){
			readNumber = 1;
			RR[2] = FR_C_RR1;
		}

		//transferencia da camada de aplicacao para o buffer	
		for (i=4, j=0; i<len-1; i++, j++){
			buffer[j]=frame_read[i];
		}
		
		if (write(port, RR, 5) < 0){
			perror("RR write");
			return -1;
		}
		readNumber = 1-readNumber;
	}
end:
	if (wrong == 1){
		if(frame_read[2]==0x00){
			readNumber = 0;
			REJ[2] = FR_C_REJ0;
		} else if (frame_read[2]==0x40){
			readNumber = 1;
			REJ[2] = FR_C_REJ1;
		}
		if (write(port, REJ, 5) < 0){
			perror("REJ write");
			return -1;
		}
	}

	//retornar comp do array lido ou erro (-1)
	return strlen(buffer);
}

int llclose(int port){

	return 0;
}
