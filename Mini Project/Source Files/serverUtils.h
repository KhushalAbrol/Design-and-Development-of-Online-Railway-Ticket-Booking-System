#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "structures.h"
#include "validate.h"

void normal_login(int);

void menu(int);


void normal_operations(int,struct database,int);
void bookTicket(int,struct database,int);
void viewBooking(int,struct database,int);
void updateBooking(int,struct database,int);
void cancleBooking(int,struct database,int);

void agent_login(int);
void agent_operations(int,struct database,int);

void admin_login(int);
void admin_operations(int);

void createAccount(int);
void modifyAccount(int);
void modifyUsername(int,int,struct database);
void modifyPassword(int,int,struct database);
void modifyAccountType(int,int,struct database);
void deleteAccount(int);
void addTrain(int);
void modifyTrain(int);
void modifyTrainName(int,int,struct trains);
void modifySeats(int,int,struct trains);
void deleteTrain(int);
void searchUser(int);
void searchTrain(int);
bool validate_train(int,int);
void displayAccounts(int);
void displayTrains(int);
struct database validate_username(char[],int);
struct database validate_account_id(int,int);
struct database validate_login_details(char[],char[],char,int);
struct trains validate_train_id(int,int);

int account_count = 0;
int train_count = 0;
struct flock lock;

void menu(int sd){
	int client_type;
	while(1){
		read(sd, &client_type, sizeof(client_type));	
		if(client_type==1){//Normal User
			normal_login(sd);
			return;
		}else if(client_type==2){//Agent
			agent_login(sd);
			return;
		}else if(client_type==3){//Admin
			admin_login(sd);
			return;	
		}
	}
}

void normal_login(int sd){
	struct database record;
	char username[25], password[25];
	bool isValid;
	
	int fd = open("Accounts.dat", O_CREAT | O_RDWR, 0666);
	
	while(1){
		read(sd, &record, sizeof(record));	
		record = validate_login_details(record.username, record.password, 'n', fd);
		lock.l_whence=SEEK_CUR;
		lock.l_start=-sizeof(record);
		lock.l_len=sizeof(record);
		lock.l_type=F_RDLCK;
		fcntl(fd, F_SETLKW, &lock);
		perror("fcntl");
		if(record.account_id != -1) isValid = true;
		else isValid = false;
		
		write(sd, &isValid, sizeof(isValid));
		
		if(isValid){
			normal_operations(sd, record, fd);
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			return;	
		}
	}
}

void normal_operations(int sd, struct database record, int fd){
	int choice;
	while(1){
		read(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:bookTicket(sd, record, fd);
				break;
			case 2:viewBooking(sd, record, fd);
				break;
			case 3:updateBooking(sd, record, fd);
				break;
			case 4:cancleBooking(sd, record, fd);
				break;
		}
	}
}

void bookTicket(int sd, struct database record, int fd){
	struct booking book;
	struct trains tr;
	int seats_available, seats_needed, booking_id;
	//If booked twice previous ticket will be cancled and new ticket will be booked.
	cancleBooking(sd, record, fd);
	int fd1 = open("Trains.dat", O_CREAT | O_RDWR, 0666);

	read(sd, &book.train_id, sizeof(book.train_id));
	tr = validate_train_id(book.train_id, fd1);

	seats_available = tr.seats_available;
	write(sd, &tr, sizeof(tr));
	if(tr.train_id==-1 || tr.isDeleted == true){
		close(fd1);
		close(fd);
		return;
	}
	read(sd, &seats_needed, sizeof(seats_needed));
	write(sd, &seats_available, sizeof(seats_available));
	printf("Needed: %d\n", seats_needed);
	printf("AVAl: %d\n", seats_available);
	if(seats_needed != -1 && seats_needed <= seats_available && seats_needed>0){
		tr.seats_available = tr.seats_available-seats_needed;
		lseek(fd1, -sizeof(tr), SEEK_CUR);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd1, &tr, sizeof(tr));
		
		book.account_id = record.account_id;
		book.isCancled = false;
		book.seats = seats_needed;
		book.book_id = (rand()%1000)+5000;
		write(sd, &book.book_id, sizeof(book.book_id));
		lseek(fd, -sizeof(struct database), SEEK_CUR);
		record.book = book;
		write(fd, &record, sizeof(record));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);		
	close(fd1);
	return;
}

void viewBooking(int sd, struct database record, int fd){
	write(sd, &record.book, sizeof(record.book));
	printf("%s", record.book.isCancled?"N":"CANCL"); 	
}

void updateBooking(int sd, struct database record, int fd){
	int seats_needed;
	struct booking book;
	if(record.book.book_id == -1){
		write(sd, "No booking found!\n", 19);
		return;
	}
	int fd1 = open("Trains.dat", O_CREAT | O_RDWR, 0666);
	struct trains tr = validate_train_id(record.book.train_id, fd1);
	if(seats_needed-record.book.seats>tr.seats_available){
		write(sd, "That much seats not available!\n", 19);
		return;
	}else{
		cancleBooking(sd, record, fd);
		read(sd, &seats_needed, sizeof(seats_needed));
		
		tr.seats_available = tr.seats_available-seats_needed;
		lseek(fd1, -sizeof(tr), SEEK_CUR);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd1, &tr, sizeof(tr));

		book.account_id = record.account_id;
		book.isCancled = false;
		book.seats = seats_needed;
		book.book_id = (rand()%1000)+5000;
		write(sd, &book.book_id, sizeof(book.book_id));
		lseek(fd, -sizeof(struct database), SEEK_CUR);
		record.book = book;
		write(fd, &record, sizeof(record));		
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
}

void cancleBooking(int sd, struct database record, int fd){
	struct trains tr;
	bool isValid;
	
	tr = validate_train_id(record.book.train_id, fd);
	int fd1 = open("Trains.dat", O_CREAT | O_RDWR, 0666);
	validate_train_id(record.book.train_id, fd1);
	if(record.book.book_id == -1 || record.book.isCancled == true){
		isValid = false;
	}else{
		isValid = true;
		lseek(fd, -sizeof(record), SEEK_CUR);
		record.book.isCancled = true;
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &record, sizeof(record));
		tr.seats_available = tr.seats_available + record.book.seats;
		lseek(fd1, -sizeof(struct trains), SEEK_CUR);
		write(fd1, &tr, sizeof(tr));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	write(sd, &isValid, sizeof(isValid));
	close(fd1);
	return;
}


void agent_login(int sd){
	struct database record;
	char username[25], password[25];
	
	bool isValid;
	int fd = open("Accounts.dat", O_CREAT | O_RDWR, 0666);
	
	while(1){	
		read(sd, &record, sizeof(record));	
		record = validate_login_details(record.username, record.password, 'a', fd);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_RDLCK;
		fcntl(fd, F_SETLKW, &lock);
		if(record.account_id != -1 && !record.isDeleted)
			isValid = true;
		else isValid = false;
		
		write(sd, &isValid, sizeof(isValid));
		
		if(isValid){
			agent_operations(sd, record, fd);
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			return;	
		}
	}
}

void agent_operations(int sd, struct database record, int fd){	int choice;
	while(1){
		read(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:bookTicket(sd, record, fd);
				break;
			case 2:viewBooking(sd, record, fd);
				break;
			case 3:updateBooking(sd, record, fd);
				break;
			case 4:cancleBooking(sd, record, fd);
		}
	}

}

void admin_login(int sd){
	struct database record;
	bool valid;
	while(1){		
		read(sd, &record, sizeof(record));
		if(!strcmp(record.username, "admin") && !strcmp(record.password, "root")){
			valid = true;			
			write(sd, &valid, sizeof(valid));
			admin_operations(sd);
			return;
		}else{
			valid = false;
			write(sd, &valid, sizeof(valid));
		}
	}
}

void admin_operations(int sd){
	int choice;
	while(1){
		read(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:createAccount(sd);
				break;
			case 2:modifyAccount(sd);
				break;
			case 3:deleteAccount(sd);
				break;
			case 4:addTrain(sd);
				break;
			case 5:modifyTrain(sd);
				break;
			case 6:deleteTrain(sd);
				break;
			case 7:searchUser(sd);
				break;
			case 8:searchTrain(sd);
				break;
			case 9:displayAccounts(sd);
				break;
			//case 10:displayTrains(sd);
			//	break;
			default:exit(0);
		}
	}
}

void createAccount(int sd){//Done
	struct database record1, record2;
	bool isValid;
	
	int fd = open("Accounts.dat", O_RDONLY|O_CREAT);
	
	read(sd, &record1, sizeof(record1));
	if(record1.account_type!='n' && record1.account_type!='a'){
		return;
	} 
	record2 = validate_username(record1.username, fd);
	if(record2.account_id != -1){
		isValid = true;
		write(sd, &isValid, sizeof(isValid));
		return;
	}else{
		isValid = false;
		write(sd, &isValid, sizeof(isValid));		
	}
	close(fd);
	fd = open("Accounts.dat", O_CREAT | O_APPEND | O_RDWR, 0666);
	if(isValid){
		return;
	}else{
		srand(time(0));
		record1.account_id = (rand()%1000)+5000;
		record1.book.book_id = -1;	
		record1.isDeleted = false;
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &record1, sizeof(record1));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		write(sd, &record1, sizeof(record1));
	}
	close(fd);
	return;
}

void modifyAccount(int sd){
	int account_id;
	bool isValid;
	struct database record;
	
	int fd = open("Accounts.dat", O_CREAT | O_RDWR, 0666);

	read(sd, &account_id, sizeof(account_id));
	record = validate_account_id(account_id, fd);
	if(record.account_id != -1){
		isValid = true;
		write(sd, &isValid, sizeof(isValid));
		int choice;
		read(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:modifyUsername(sd, fd, record);
				break;
			case 2:modifyPassword(sd, fd, record);
				break;
			case 3:modifyAccountType(sd, fd, record);
				break;
		
		}
	}else{
		isValid = false;
		write(sd, &isValid, sizeof(isValid));
	}
	close(fd);
}

void modifyUsername(int sd, int fd, struct database record){
	struct database record2; 
	bool isValid;

	read(sd, record.username, sizeof(record.username));
	record2 = validate_username(record.username, fd);
	
	if(record2.account_id != -1){
		isValid = true;
		write(sd, &isValid, sizeof(isValid));
	}else{
		isValid = false;
		lseek(fd, -sizeof(struct database), SEEK_CUR);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &record, sizeof(record));
		write(sd, &isValid, sizeof(isValid));
	}
	close(fd);
}

void modifyPassword(int sd, int fd, struct database record){
	read(sd, record.password, sizeof(record.password));
	lseek(fd, -sizeof(struct database), SEEK_CUR);
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	write(fd, &record, sizeof(struct database));
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void modifyAccountType(int sd, int fd, struct database record){
	int choice;
		
	write(sd, &record, sizeof(record));
	read(sd, &choice, sizeof(choice));
	if(choice == 1){
		char type = record.account_type;
		if(type=='n')
			record.account_type = 'a';
		else
			record.account_type = 'n';
	}
	lseek(fd, -sizeof(struct database), SEEK_CUR);
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	write(fd, &record, sizeof(record));
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void deleteAccount(int sd){
	struct database record;
	bool isValid;

	int fd = open("Accounts.dat", O_CREAT | O_RDWR, 0666);
	read(sd, record.username, sizeof(record.username));
	record = validate_username(record.username, fd);
	
	if(record.account_id != -1){
		isValid = true;
	}else{
		isValid = false;
	}
	write(sd, &isValid, sizeof(isValid));
	if(isValid){
		record.isDeleted = true;
		lseek(fd, -sizeof(record), SEEK_CUR);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &record, sizeof(record));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);
}

void addTrain(int sd){
	struct trains newTrain;
	char buffer[50];
	bool isValid;
		
	int fd = open("Trains.dat", O_CREAT | O_RDONLY, 0666);	
	read(sd, &newTrain, sizeof(newTrain));	
	isValid = validate_train(newTrain.train_id, fd);
	
	printf("%s", newTrain.name);
	close(fd);
	fd = open("Trains.dat", O_CREAT | O_APPEND | O_RDWR, 0666);	
	write(sd, &isValid, sizeof(isValid));
	if(!isValid){
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &newTrain, sizeof(newTrain));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);
}

void modifyTrain(int sd){
	int train_id;
	bool isValid;
	struct trains tr;
	
	int fd = open("Trains.dat", O_CREAT | O_RDONLY, 0666);
	read(sd, &train_id, sizeof(train_id));
	tr = validate_train_id(train_id, fd);
	if(tr.train_id != -1){
		isValid = true;
	}else{
		isValid = false;
	}
	write(sd, &isValid, sizeof(isValid));
	if(isValid){
		int choice;
		read(sd, &choice, sizeof(choice));
		
		while(1){
			read(sd, &choice, sizeof(choice));
			switch(choice){
				case 1:modifyTrainName(sd, fd, tr);
					return;
				case 2:modifySeats(sd, fd, tr);
					return;
			}
		}
	}
}


void modifyTrainName(int sd, int fd, struct trains tr){
	read(sd, tr.name, sizeof(tr.name));
	lseek(fd, -sizeof(struct trains), SEEK_CUR);
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	write(fd, &tr, sizeof(struct trains));
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void modifySeats(int sd, int fd, struct trains tr){	
	read(sd, &tr.seats_available, sizeof(tr.seats_available));	
	lseek(fd, -sizeof(struct trains), SEEK_CUR);
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	write(fd, &tr, sizeof(struct trains));
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void deleteTrain(int sd){
	struct trains tr;
	bool isValid;
	int choice, train_id;
	
	int fd = open("Trains.dat", O_CREAT | O_RDWR, 0666);
	
	read(sd, &train_id, sizeof(train_id));
	tr = validate_train_id(train_id, fd);
	
	if(tr.train_id==-1 || tr.isDeleted){
		isValid = false;
	}else{
		isValid = true;
	}
	write(sd, &isValid, sizeof(isValid));
	if(isValid){
		tr.isDeleted = true;
		lseek(fd, -sizeof(struct trains), SEEK_CUR);
		lock.l_whence=SEEK_SET;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_type=F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		write(fd, &tr, sizeof(tr));
		lock.l_type=F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
}

void searchUser(int sd){//Done
	struct database record;
	int fd = open("Accounts.dat", O_CREAT | O_RDONLY, 0666);
	read(sd, record.username, sizeof(record.username));
	record = validate_username(record.username, fd);
	write(sd, &record, sizeof(record));	
}

void searchTrain(int sd){
	struct trains tr;
	int train_id;
	
	int fd = open("Trains.dat", O_CREAT | O_RDONLY, 0666);
	read(sd, &train_id, sizeof(train_id));
	tr = validate_train_id(train_id, fd);
	write(sd, &tr, sizeof(tr));
}

void displayAccounts(int sd){
	struct database record;
	int fd = open("Accounts.dat", O_CREAT | O_RDONLY, 0666);
	lseek(fd, 0, SEEK_SET);
	while(read(fd, (char *)&record, sizeof(struct database))){
		write(sd, &record, sizeof(record));
	}
	record.account_id=-1;
	write(sd, &record, sizeof(record));
	close(fd);
}

/*
void displayTrains(int sd){
	struct trains tr;
	int fd = open("Trains.dat", O_CREAT | O_RDONLY, 0666);
	while(read(fd, &tr, sizeof(struct database))){
		write(sd, &tr, sizeof(tr));
	}
	tr.train_id=-1;
	write(sd, &tr, sizeof(tr));
	close(fd);
}
*/
