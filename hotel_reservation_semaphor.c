/*
	Name: Hai Tran
	Peoplesoft ID: 1056042
	Class: COSC 4330 - Operating System
*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <math.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define my_key "1056042"

typedef struct room
{
	char begin_date[10];
	int num_date;
	char customer_name[30];
}room_t;

typedef struct customer
{
	char name[30];				// name customer
	int final_room[100];		//final rooms 
	int total_room;				// total final rooms
	int room[30][30];			// room for each operation
	char date[30][15];			// date for reserve or cancel
	int num_date[30];			// total days for reserve
	char operation[30][15];		// type operation
	int num_operation;			// total operation
	int reserve_time;
	int cancel_time;
	int check_time;
}customer_t;

	int max_room = 0;
	int max_customer = 0;

void get_max_room_customer (int *max_room, int *max_customer)
{
	int count = 0;
	int num;
	FILE *fp;
	fp = fopen ("Input.txt","r");

	while (count < 2 && !feof(fp))
	{
		fscanf(fp,"%d\n", &num);
		if ( count == 0)
			*max_room = num;
		else
			*max_customer = num;
		++ count;
	}
	fclose(fp);
}

void split(char src[], char dest[][20])
{
	char *prt;
	int i = 0;
	prt = strtok (src," ");
	while (prt != NULL)
	{
		strcpy(dest[i], prt);
		++i;
		prt = strtok(NULL," ");
	}

}

void get_room_number(char src[], int dest[])
{
	int x =1;

	while (isdigit(src[x]))
	{
			++x;
	}

	if (src[x] == '-')
	{
		int a[2] = {0}, i = 0;
		char *pt;
		pt = strtok(src,"()-");
		while (pt != NULL)
		{
			a[i] = atoi (pt);
			++i;
			pt = strtok(NULL,"()- ");
		}
		int j = 0;
		for ( a[0]; a[0]<= a[1]; ++ a[0])
		{
			dest[j] = a[0];
			++j;
		}
	}
	else
	{
		if (src[x] ==',')
		{
			int a[] ={0}, i = 0;
			char *pt;
			pt = strtok(src,"(),");
			while (pt != NULL)
			{
				a[i] = atoi (pt);
				++i;
				pt = strtok(NULL,"(),");
			}
			int j;
			for (j =0; j < i;++j)
			{
				dest[j] = a[j];
			}	
		}
		else
		{
			char *pt="";
			pt = &src[1];
			dest[0] = atoi (pt);
		}
	}
	
}
void get_customer (int max_customer, customer_t customer[])
{
	int start_time = 0;					// the line is critical time
	int start_op = 0;					// the line is for operation of customer
	int count_time	= 0;				// countter for # of critical time (<=3)
	int count = 0;						// count number of customer	
	
	FILE *fp;
	fp = fopen("Input.txt","r");
	while (count < max_customer && !feof(fp))
	{
		char data[100] = "";				// take each line of Input.txt
		fgets (data,100,fp);
		//printf ("%s\n",data);
		
		if (strncmp(data,"customer",8) == 0)
		{
			strncpy(customer[count].name,data,strlen(data) -1);
			customer[count].num_operation = 0;
			start_time = 1;
		}
		else
		{
			if (start_time == 1)
			{
				int time = 0;
				char *time_char ="";
				switch (count_time)
				{
					case 0 :
						time_char = &data[7];
						time = atoi(time_char);
						customer[count].reserve_time = time;
						break;
					case 1 :
						time_char = &data[6];
						time = atoi(time_char);
						customer[count].cancel_time = time;
						break;
					case 2 :
						time_char = &data[5];
						time = atoi(time_char);
						customer[count].check_time = time;
						start_time = 0;
						start_op = 1;
						break;
				}
				++count_time;
			}
			else
			{
				count_time = 0;
				if (start_op == 1)
				{
					if (strncmp(data,"end",3) != 0)
					{
						char op[4][20]={"","","",""};
						split (data, op);
						bzero(customer[count].date[customer[count].num_operation], 10);
						//printf ("%s		%s		%s		%s\n", op[0],op[1],op[2],op[3]);
						strcpy (customer[count].operation[customer[count].num_operation], op[0]);		//get type operation
						strcpy (customer[count].date[customer[count].num_operation], op[2]);
						customer[count].num_date[customer[count].num_operation] = atoi(op[3]);
						get_room_number(op[1],customer[count].room[customer[count].num_operation]);
						++ customer[count].num_operation;
					}
					else
					{
						start_op = 0; 
						++count;
					}
				}
			}
		}
	}
	fclose(fp);

}

room_t init_room()
{
	room_t room;
	strcpy (room.begin_date,"");
	strcpy (room.customer_name,"");
	room.num_date = 0;
	return room;
}

void reserve (room_t *room, customer_t customer[], int count, int index, sem_t *room_sem[],FILE *fp)
{
	int i;
	int found = 0;
	int room_count = 0;
	int sem_value = 0;
	for (i =0;i <30; ++i)
	{
		if (customer[count].room[index][i] != 0)
		{
			if (customer[count].room[index][i] > max_room)
			{
				found = 1;
				//printf("	reserve fail because the room %d doesn't exist\n",customer[count].room[index][i]);
				fprintf(fp,"	reserve fail because the room %d doesn't exist\n",customer[count].room[index][i]);
				break;
			}
			else
			{
				++room_count;
			}
		}
		else
		{
			break;
		}

	}
	if (found == 0)
	{
		for (i = 0; i< room_count;++i)
		{
			sem_getvalue(room_sem[customer[count].room[index][i]], &sem_value);
			if (sem_value == 0)
			{
				found = 1;
				//printf("	reserve is fail because room %d is taken\n",customer[count].room[index][i]);
				fprintf(fp,"	reserve is fail because room %d is taken\n",customer[count].room[index][i]);
				break;
			}		
		}
		if (found == 0)
		{
			for ( i =0; i< room_count;++i)
			{
				sem_wait(room_sem[customer[count].room[index][i]]);
				strcpy(room[customer[count].room[index][i]].begin_date,customer[count].date[index]);
				room[customer[count].room[index][i]].num_date = customer[count].num_date[index];
				strcpy(room[customer[count].room[index][i]].customer_name, customer[count].name);
				customer[count].final_room[customer[count].total_room] = customer[count].room[index][i];
				++customer[count].total_room;
				//printf("		room %d		%s		%d\n",customer[count].room[index][i], customer[count].date[index], room[customer[count].room[index][i]].num_date);
				fprintf(fp,"		room %d		%s		%d\n",customer[count].room[index][i], customer[count].date[index], room[customer[count].room[index][i]].num_date);
				//sem_wait(room_sem[customer[count].room[index][i]]);
			}
		}
	}
}

void cancel (room_t *room, customer_t customer[], int count, int index, sem_t *room_sem[],FILE *fp)
{
	int i;
	int room_count = 0;
	int sem_value = 0;
	for (i = 0; i<30;++i)
	{
		if (customer[count].room[index][i] != 0)
		{
			if(customer[count].room[index][i] > max_room)
			{	
				//printf("	cancel fail because the room %d doesn't exist\n",customer[count].room[index][i]);
				fprintf(fp,"	cancel fail because the room %d doesn't exist\n",customer[count].room[index][i]);
				break;
			}
			else
			{
				sem_getvalue (room_sem[customer[count].room[index][i]], &sem_value);
				if (sem_value == 0)
				{
					sem_post (room_sem[customer[count].room[index][i]]);
					if (strcmp(customer[count].name, room[customer[count].room[index][i]].customer_name) != 0)
					{
						//printf("	room %d is not yours, %s\n", customer[count].room[index][i],customer[count].name);
						fprintf(fp,"	room %d is not yours, %s\n", customer[count].room[index][i],customer[count].name);
						sem_wait(room_sem[customer[count].room[index][i]]);
					}
					else
					{
						if (customer[count].num_date[index] >= room[customer[count].room[index][i]].num_date)
						{
							room[customer[count].room[index][i]] = init_room();
							//printf("	room %d cancel\n", customer[count].room[index][i]);
							fprintf(fp,"	room %d cancel\n", customer[count].room[index][i]);
						}
						else
						{
							room[customer[count].room[index][i]].num_date -= customer[count].num_date[index];
							//printf("	room %d cancel %d days\n",customer[count].room[index][i],customer[count].num_date[index]);
							fprintf(fp,"	room %d cancel %d days\n",customer[count].room[index][i],customer[count].num_date[index]);
							sem_wait(room_sem[customer[count].room[index][i]]);
						}
					}
				}
				else
				{
					//printf("	room %d has not reservedd yet\n",customer[count].room[index][i]);
					fprintf(fp,"	room %d has not reservedd yet\n",customer[count].room[index][i]);
				}
			}
		}
		else
		{
			break;
		}
	}

}
void check (room_t *room, customer_t customer[], int count,sem_t *room_sem[],FILE *fp)
{
	int i;
	int sem_value = 0;
	for (i = 0; i <customer[count].total_room;++i)
	{
		
		if (customer[count].final_room[i] != 0)
		{
			sem_getvalue (room_sem[customer[count].final_room[i]], &sem_value);
			if (sem_value == 0)
			{
				//printf("	room %d		%s		%d\n", customer[count].final_room[i],room[customer[count].final_room[i]].begin_date, room[customer[count].final_room[i]].num_date);
				fprintf(fp,"	room %d		%s		%d\n", customer[count].final_room[i],room[customer[count].final_room[i]].begin_date, room[customer[count].final_room[i]].num_date);
			}
		}
	}
}

int main()
{
	get_max_room_customer(&max_room, &max_customer);		// get number of room and customer
	//printf ("%d - %d\n",max_room, max_customer);

	customer_t customer[max_customer];				// declare a new structure customer
	get_customer (max_customer, customer);				// take customer from Input.txt
	//----------- share memory -------------------
	key_t key = ftok ("1056042", 'R');
	int shmid;
	room_t *room;
	if ((shmid = shmget (key, 512, 0644 | IPC_CREAT)) <0)
	{
		perror ("Shmget error");
		exit (1);
	}
	room = shmat (shmid, (void *)0,0);
	int i;
	for (i =1;i<= max_room;++i)
	{
		room[i] = init_room();
	}

	//---------- semaphore and fork()-------------------
	sem_t *room_sem[max_room+1];
	char room_key[10] ="";
	for (i = 1; i <= max_room; ++i)
	{
		sprintf(room_key,"%s_%d",my_key,i);
		room_sem[i] = sem_open(room_key,O_CREAT,0600,1);
		if (sem_init(room_sem[i],1,1) < 0)
		{
			printf ("semaphore at room %d error\n",i);
			exit(1);
		}
	}
	int count;
	for (count = 0; count < max_customer; ++count)
	{
		if (fork() == 0)
		{
			FILE *fp;
			int j;
			 for (j = 0;j <customer[count].num_operation; ++j)
			 {
				 fp= fopen("Output.txt","a");
				 if (strcmp(customer[count].operation[j], "reserve") == 0)
				 {
					 //printf("%s reserve:\n",customer[count].name);
					 fprintf(fp,"%s reserve:\n",customer[count].name);
					 reserve (room,customer,count,j, room_sem,fp);
					 usleep(customer[count].reserve_time*1000);
				 }
				 else
				 {
					 if (strcmp(customer[count].operation[j], "cancel") == 0)
					 {
					      //printf("%s cancel:\n",customer[count].name);
						  fprintf(fp,"%s cancel:\n",customer[count].name);
						  cancel (room,customer,count,j, room_sem,fp);
						  usleep(customer[count].cancel_time*1000);
					 }
					 else
					 {
						 
						//printf("%s check:\n",customer[count].name);
						fprintf(fp,"%s check:\n",customer[count].name);
						check(room,customer,count,room_sem,fp);
						usleep(customer[count].check_time*1000);
					 }
				 }
				 fclose(fp);
			 }
			 _exit(0);
		}
	}
	for (count = 0; count < max_customer; ++count)
	{
		wait(NULL);
	}
	for ( i = 1; i< max_room;++i)
	{
		sem_destroy (room_sem[i]);
	}
	shmdt(room);
	shmctl(shmid,IPC_RMID, NULL);	
	return 0;
}
