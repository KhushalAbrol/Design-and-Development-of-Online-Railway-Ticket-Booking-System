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


/*
Validate functions is a group of functions which will 
search the given parameter in the dataset and return the
record if found, else return -1 in id of that struct
*/

struct flock lock;

struct database validate_login_details(char username[], char password[], char account_type, int fd){
	struct database record;	
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_RDLCK;
	fcntl(fd, F_SETLKW, &lock);
	while(read(fd, &record, sizeof(struct database))){
		printf("%s %s\n", record.username, record.password);	
		if(!strcmp(record.username, username) && !strcmp(record.password, password) && record.account_type == account_type){
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);	
			return record;
		}		
	}
	record.account_id=-1;
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return record;
}

//returns true if username exists
struct database validate_username(char username[], int fd){
	struct database record;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_RDLCK;
	fcntl(fd, F_SETLKW, &lock);
	while(read(fd, &record, sizeof(struct database))){
		if(!strcmp(record.username, username) && record.isDeleted == false){			
			printf("%s", record.username);	
			return record;
		}
	}
	record.account_id=-1;
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return record;
}

//return true if account id exists
struct database validate_account_id(int account_id, int fd){
	struct database record;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd, F_SETLKW, &lock);
	while(read(fd, (char*)&record, sizeof(struct database)))
		if(record.account_id == account_id && record.isDeleted == false)
			return record;
	record.account_id=-1;
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return record;
}

bool validate_train(int train_id, int fd){
	struct trains newTrain;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_RDLCK;
	fcntl(fd, F_SETLKW, &lock);
	while(read(fd, (char*)&newTrain, sizeof(struct trains))){
		if(newTrain.train_id == train_id && newTrain.isDeleted == false){
			lock.l_type=F_UNLCK;
			fcntl(fd, F_SETLK, &lock);		
			return true;
		}
	}
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return false;
}

struct trains validate_train_id(int train_id, int fd){
	struct trains tr;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd, F_SETLKW, &lock);
	while(read(fd, (char*)&tr, sizeof(struct trains))){
		if(tr.train_id == train_id && tr.isDeleted == false)
			return tr;
		tr.train_id=-1;
	}
	lock.l_type=F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return tr;
}
