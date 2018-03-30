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
#define MAXBUF 4096

/*Flags*/
#define FR_F     0x7e    //0111 1110
#define FR_F_AUX 0x7d5e  //0111 1101 0101 1110

#define ESC         0x7d    //0111 1101
#define ESC_AUX     0x7d5d  //0111 1101 0101 1101 

#define FR_A_RX     0x03    //0000 0011 
#define FR_A_TX     0x01    //0000 0001

#define FR_C_SET    0x03    //0000 0011
#define FR_C_DISC   0x0B    //0000 1011
#define FR_C_UA     0x07    //0000 0111
#define FR_C_RR     0x05    //0000 0101
#define FR_C_REJ    0x01    //0000 0001

int llopen(int port, int MODE);

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[MAXBUF];
	unsigned char SET[MAXBUF], frame_read[MAXBUF];
    int i, sum = 0, speed = 0;

    
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
		return -1;
	}
	if(strncmp(buf,"TX", 2) == 0){
		llopen(fd, TX);
	} else if(strncmp(buf, "RX", 2) ==0) {
		llopen(fd, RX);
	} else {
		perror("Bad input: Use 'RX' or 'TX' as an argument");
	}

	/*
	printf("String?\n");
	if( gets(buf) == 0) {  //get string from terminal input
		perror("gets");
		return -1;
	}
	*/

	/*
    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }
	*/
    
    /*testing*/
    //buf[254] = '\0';
    
    //res = write(fd, buf,255);
	//res = write(fd, frame, strlen(frame)+1);   
    //printf("%d bytes written\n", res);
 

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
	unsigned char frame_read[MAXBUF];
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
