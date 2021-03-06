/* lab7.c
 * Basil Lin
 * basill
 * ECE 2220, Fall 2016
 * MP7
 *
 * Purpose: Practice handling processes and handling signals by writing a program that models 
 * mission control in guiding three space crafts to Mars and back to Earth
 *
 * Assumptions: Four terminal windows are open to start the program. The user will enter the 
 * correct terminals in the command line. The user will also enter correct commands.
 *
 * Command line arguments: four terminal window numbers, with the first terminal window 
 * representing mission control, and the other three representing space craft
 *
 * Notes: I installed a kill handler signal in order to exit with an error when a kill command
 * is used, which is needed for the final print.
 *   
 * Known bugs: None known
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define NUMTTYS 4
#define MAXLINE 4
#define STRDEVMAX 500

void sighandler1 (int signum);
void sighandler2 (int signum);
void sighandler3 (int signum);
void killhandler (int signum);

FILE *fpt;
int distance = 0, rover = 1, clicks = 0, wptflag = -1;
float wpt = 10.5;

int main(int argc, char *argv[])
{

	int sc1 = -1, sc2 = -1, sc3 = -1, collect = -1, i;
	char strDev1[STRDEVMAX], strDev2[STRDEVMAX], strDev3[STRDEVMAX];
	char command[MAXLINE], line[MAXLINE];
	
	/* creates printing setup */
	sprintf(strDev1, "/dev/pts/%d", atoi(argv[2]));
	sprintf(strDev2, "/dev/pts/%d", atoi(argv[3]));
	sprintf(strDev3, "/dev/pts/%d", atoi(argv[4]));
	
	/* forks off space craft and collect child process */
	sc1 = fork();
	if (sc1 > 0) {
		sc2 = fork();
	}	
	if (sc2 > 0) {
		sc3 = fork();
	}   
	if (sc3 > 0) {
		collect = fork();
	}
	
	if (collect == 0) // fourth child process, collects commands and distributes signals
	{
		printf("\n\nHere are the following commands:\n\n");
		printf("ln - Instructs space craft n to launch its Rover. n is one of 1, 2, or 3.\n");
		printf("kn - Instruct space craft n to cancel its mission. n is 1, 2, or 3.\n");
		printf("tn - Transmit 10 new way points to space craft n.\n");
		printf("q - Exits the program.\n\n");
		
		while(1) { // keeps checking for input
			fgets(line, MAXLINE, stdin);
        	sscanf(line, "%s", command);
        	if (!strcmp(command, "q")) { // quit
        		printf("\nExiting.\n\n");
        		kill(sc1, SIGTERM);
        		kill(sc2, SIGTERM);
        		kill(sc3, SIGTERM);	
        		kill(collect, SIGTERM);
        		kill(getppid(), SIGTERM);
		    } else if (command[1] == '1' && strlen(command) == 2) { // send to Space Craft 1
		    	if (command[0] == 'l') {
		    		kill(sc1, SIGUSR1); // launch Rover
		    	} else if (command[0] == 'k') {
		    		kill(sc1, SIGTERM); // kill Rover
		    		printf("Terminated Space Craft 1\n");
		    	} else if (command[0] == 't') {
		    		kill(sc1, SIGUSR2); // transmits way points to Rover
		    	}
		    } else if (command[1] == '2' && strlen(command) == 2) { // send to Space Craft 2
		    	if (command[0] == 'l') {
		    		kill(sc2, SIGUSR1); // launch Rover
		    	} else if (command[0] == 'k') {
		    		kill(sc2, SIGTERM); // kill Rover
		    		printf("Terminated Space Craft 2\n");
		    	} else if (command[0] == 't') {
		    		kill(sc2, SIGUSR2); // transmits way points to Rover
		    	}
		    } else if (command[1] == '3' && strlen(command) == 2) { // send to Space Craft 3
		    	if (command[0] == 'l') {
		    		kill(sc3, SIGUSR1); // launch Rover
		    	} else if (command[0] == 'k') {
		    		kill(sc3, SIGTERM); // kill Rover
		    		printf("Terminated Space Craft 3\n");
		    	} else if (command[0] == 't') {
		    		kill(sc3, SIGUSR2); // transmits way points to Rover
				}
		    }
		}
	} else if (sc3 == 0) { // Space Craft 3
	
		/* install signal handlers */
		signal(SIGUSR1, sighandler1);
		signal(SIGUSR2, sighandler2);
		signal(SIGALRM, sighandler3);
		signal(SIGTERM, killhandler);
		
		srand48(time(NULL) * 3);
		while (clicks < 30 || clicks >= 60) { // chooses random clicks from [30, 60)
			clicks = 100 * drand48();
		}
		
		fpt = fopen(strDev3, "w");
		fprintf(fpt, "\n\nSpace Craft 3 initiating mission. Distance to Mars %d\n\n", clicks);
		
		while(1) {
			alarm(1);
			
			/* Space Craft printing */
        	fprintf(fpt, "\n\n WAYPOINTS:\t%d\n", (int)wpt);
        	if (rover == 1) {
        		fprintf(fpt, " ROVER:\t\tNOT LAUNCHED\n");
        	} else {
        		fprintf(fpt, " ROVER:\t\tLAUNCHED\n");
        	}
        	fprintf(fpt, "\n DISTANCE TO MARS: %d\n       ", clicks - distance);
        	for (i = 0; i < distance; i++) {
        		fprintf(fpt, "-");
        	}
        	if (rover == 1) {
        		fprintf(fpt, ")==>\n EARTH ");
        	} else {
        		fprintf(fpt, "<==(\n EARTH ");
        	}
        	for (i = 0; i < clicks; i++) {
        		fprintf(fpt, "-");
        	}
        	fprintf(fpt, " MARS\n");
        	fprintf(fpt, "\n_______________________________________________________________________\n");
        	
			if (distance == 0 && rover == 0 && wpt >= 0.5) { // successful return
		    	exit(0);
		    }
		    if (distance == clicks) { // entered orbit
		    	fprintf(fpt, "\nSpace craft 3 to Mission Control: entered orbit. Please signal when to launch rover.\n");
		    }
		    if (wpt == 4.5) { // low waypoints
		    	fprintf(fpt, "\nHouston, we have a problem!\n");
		    } else if (wpt == 0.5) { // out of waypoints
		    	fprintf(fpt, "\nSpace craft 3. Lost in Space.\n");
		    	exit(1);
		    }
		    pause();
		}
	} else if (sc2 == 0) { // Space Craft 2
		
		/* install signal handlers */
		signal(SIGUSR1, sighandler1);
		signal(SIGUSR2, sighandler2);
		signal(SIGALRM, sighandler3);
		signal(SIGTERM, killhandler);
		
		srand48(time(NULL) * 2);
		while (clicks < 30 || clicks >= 60) { // chooses random clicks from [30, 60)
			clicks = 100 * drand48();
		}
		
		fpt = fopen(strDev2, "w");
		fprintf(fpt, "\n\nSpace Craft 2 initiating mission. Distance to Mars %d\n\n", clicks);
		
		while(1) {
			alarm(1);
        	
        	/* Space Craft printing */
        	fprintf(fpt, "\n\n WAYPOINTS:\t%d\n", (int)wpt);
        	if (rover == 1) {
        		fprintf(fpt, " ROVER:\t\tNOT LAUNCHED\n");
        	} else {
        		fprintf(fpt, " ROVER:\t\tLAUNCHED\n");
        	}
        	fprintf(fpt, "\n DISTANCE TO MARS: %d\n       ", clicks - distance);
        	for (i = 0; i < distance; i++) {
        		fprintf(fpt, "-");
        	}
        	if (rover == 1) {
        		fprintf(fpt, ")==>\n EARTH ");
        	} else {
        		fprintf(fpt, "<==(\n EARTH ");
        	}
        	for (i = 0; i < clicks; i++) {
        		fprintf(fpt, "-");
        	}
        	fprintf(fpt, " MARS\n");
        	fprintf(fpt, "\n_______________________________________________________________________\n");
        	
		    if (distance == 0 && rover == 0 && wpt >= 0.5) { // successful return
		    	exit(0);
		    }
		    if (distance == clicks) { // entered orbit
		    	fprintf(fpt, "\nSpace craft 2 to Mission Control: entered orbit. Please signal when to launch rover.\n");
		    }
		    if (wpt == 4.5) { // low waypoints
		    	fprintf(fpt, "\nHouston, we have a problem!\n");
		    } else if (wpt == 0.5) { // out of waypoints
		    	fprintf(fpt, "\nSpace craft 2. Lost in Space.\n");
		    	exit(1);
		    }
		    pause();
        }
	} else if (sc1 == 0) { // Space Craft 1
	
		/* install signal handlers */
		signal(SIGUSR1, sighandler1);
		signal(SIGUSR2, sighandler2);
		signal(SIGALRM, sighandler3);
		signal(SIGTERM, killhandler);
		
		srand48(time(NULL));
		while (clicks < 30 || clicks >= 60) { // chooses random clicks from [30, 60)
			clicks = 100 * drand48();
		}
		
		fpt = fopen(strDev1, "w");
		fprintf(fpt, "\n\nSpace Craft 1 initiating mission. Distance to Mars %d\n\n", clicks);
		
		while(1) {
			alarm(1);
        	
        	/* Space Craft printing */
        	fprintf(fpt, "\n\n WAYPOINTS:\t%d\n", (int)wpt);
        	if (rover == 1) {
        		fprintf(fpt, " ROVER:\t\tNOT LAUNCHED\n");
        	} else {
        		fprintf(fpt, " ROVER:\t\tLAUNCHED\n");
        	}
        	fprintf(fpt, "\n DISTANCE TO MARS: %d\n       ", clicks - distance);
        	for (i = 0; i < distance; i++) {
        		fprintf(fpt, "-");
        	}
        	if (rover == 1) {
        		fprintf(fpt, ")==>\n EARTH ");
        	} else {
        		fprintf(fpt, "<==(\n EARTH ");
        	}
        	for (i = 0; i < clicks; i++) {
        		fprintf(fpt, "-");
        	}
        	fprintf(fpt, " MARS\n");
        	fprintf(fpt, "\n_______________________________________________________________________\n");
        	
		    if (distance == 0 && rover == 0 && wpt >= 0.5) { // succesfull return
		    	exit(0);
		    }
		    if (distance == clicks) { // entered orbit
		    	fprintf(fpt, "\nSpace craft 1 to Mission Control: entered orbit. Please signal when to launch rover.\n");
		    }
		    if (wpt == 4.5) { // low waypoints
		    	fprintf(fpt, "\nHouston, we have a problem!\n");
		    } else if (wpt == 0.5) { // out of waypoints
		    	fprintf(fpt, "\nSpace craft 1. Lost in Space.\n");
		    	exit(1);
		    }
		    pause();
        }
	}
	
	if (sc1 > 0 && sc2 > 0 && sc3 > 0 && collect > 0) // parent function
	{
		/* finds exit statuses */
		int status1, status2, status3, pid1, pid2, pid3, stat1, stat2, stat3;
		pid1 = wait(&status1);
		pid2 = wait(&status2);
		pid3 = wait(&status3);
		status1 >>= 8;
		status2 >>= 8;
		status3 >>= 8;
		
		/* assigns correct exit statuses */
		if (pid1 == sc1) {
			stat1 = status1;
		} else if (pid1 == sc2) {
			stat2 = status1;
		} else if (pid1 == sc3) {
			stat3 = status1;
		}
		
		if (pid2 == sc1) {
			stat1 = status2;
		} else if (pid2 == sc2) {
			stat2 = status2;
		} else if (pid2 == sc3) {
			stat3 = status2;
		}
		
		if (pid3 == sc1) {
			stat1 = status3;
		} else if (pid3 == sc2) {
			stat2 = status3;
		} else if (pid3 == sc3) {
			stat3 = status3;
		}
		
		/* prints messages based on exit status */
		if (stat1 == 1) {
			printf("\nVaya Con Dios, Space craft 1\n");
		} else if (stat1 == 0) {
			printf("\nWelcome home, Space craft 1\n");
		}
	
		if (stat2 == 1) {
			printf("\nVaya Con Dios, Space craft 2\n");
		} else if (stat2 == 0) {
			printf("\nWelcome home, Space craft 2\n");
		}
	
		if (stat3 == 1) {
			printf("\nVaya Con Dios, Space craft 3\n");
		} else if (stat3 == 0) {
			printf("\nWelcome home, Space craft 3\n");
		}
		
		if (stat1 == 0 && stat2 == 0 && stat3 == 0) {
			printf("\n\nCongratulations team: Mission successful\n\n");
		} else {
			printf("\n\nMission failed\n\n");
		}
	}
	
	kill(collect, SIGTERM); // kills collection child process
	return 0;
}
	
/* Rover Launcher
 * Inputs: distance, clikcs, and rover variables.
 * Outputs: new value for rover variable
 * Purpose: launches rover if space craft is in orbit, otherwise,
 * self destructs due to command error.
 */
void sighandler1 (int signum) {
	if (distance == clicks && rover == 1) {
		rover--;
	} else {
		fprintf(fpt, "\nCommand Error: Self Destructing...\n");
		kill(getpid(), SIGTERM);
	}
}

/* Waypoint Adder
 * Inputs: wpt variable (number of waypoints)
 * Outputs: updated wpt
 * Purpose: adds 10 more waypoints when called
 */
void sighandler2 (int signum) {
	wpt += 10;
}

/* Distance Incrementer / Decrementer
 * Inputs: wpt variable (number of waypoints), distance variable
 * Outputs: updated distance variable
 * Purpose: increments distance every second if traveling to Mars,
 * does nothing if in orbit of Mars, decrements distance every second
 * if traveling back to Earth.
 */
void sighandler3 (int signum) {
	wpt -= 0.5;
	if (rover == 0) {
		distance -= 1;
	} else if (distance == clicks) {
		distance = distance;
	} else {
		distance += 1;
	}
}

/* Kill Handler
 * Inputs: none
 * Ouputs: none
 * Purpose: adds an error exit code when kill commands are called,
 * also completes kill command.
 */
void killhandler (int signum) {
	kill(getpid(), SIGTERM);
	exit(1);
}
	
	
	

