#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "structures.h"

void menu(int);
void normal_login(int);
struct database validate_login_details(char[],char[],char);
void normal_operations(int);
void bookTicket(int);
void viewBooking(int);
void updateBooking(int);
void cancleBooking(int);
void agent_login(int);
void agent_operations(int);
void admin_login(int);
void admin_operations(int);
void createAccount(int);
struct database validate_username(char*,int);
void modifyAccount(int);
struct database validate_account_id(int,int);
void modifyUsername(int);
void modifyPassword(int);
void modifyAccountType(int);
void deleteAccount(int);
void addTrain(int);
bool validate_train(int,int);
void modifyTrain(int);
struct trains validate_train_id(int);
void modifyTrainName(int);
void modifySeats(int);
void deleteTrain(int);
void searchUser(int);
void searchTrain(int);
void displayAccounts(int);
void displayTrains(int);

void menu(int sd){
	int client_type;
	printf("Select user type:\n");
	printf("1: Normal User\n2: Agent User\n3: Administrator\n");
	while(1){
		printf("Enter your choice: ");
		scanf("%d", &client_type);		
		write(sd, &client_type, sizeof(client_type));
		if(client_type==1){//Normal User
			normal_login(sd);
			return;
		}else if(client_type==2){//Agent
			agent_login(sd);
			return;
		}else if(client_type==3){//Admin
			admin_login(sd);
			return;	
		}else{
			printf("Invalid user type!\n Please enter user type again: \n");		
		}
	}
}

void normal_login(int sd){
	struct database record;
	bool isValid;
		
	while(1){
		printf("Enter username: ");
		scanf("%s", record.username);
		printf("Password: ");
		scanf("%s", record.password);
		
		write(sd, &record, sizeof(record));
		read(sd, &isValid, sizeof(isValid));
		
		if(isValid){
			normal_operations(sd);
			return;
		}
		else
			printf("Invalid Username or Password! Please try again\n");			
	} 
}

void normal_operations(int sd){	
	int choice;
	system("clear");
	while(1){
		printf("\n*************************** Welcome User ***************************\n\n");				
		printf("Select the operation you want to perform:\n1: Book train ticket\n2: View Booking\n3: Update Booking \n4: Cancel Booking\nElse: Go back\n");
		printf("Select the option: ");
		scanf("%d", &choice);
		write(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:bookTicket(sd);
			 	break;
			case 2:viewBooking(sd);
				break;
			case 3:updateBooking(sd);
				break;
			case 4:cancleBooking(sd);
				break;
			default:printf("Invalid Choice!\n");
		}
		printf("Press enter to continue...\n");
		getchar();
		getchar();
		system("clear");
	}
}

void bookTicket(int sd){
	struct booking book;
	struct trains tr;
	int seats_available, seats_needed;
	int booking_id;
	
	printf("Enter the details to book the tickets:\n");
	printf("Enter train number: ");
	scanf("%d", &book.train_id);
	write(sd, &book.train_id, sizeof(book.train_id));
	read(sd, &tr, sizeof(tr));
	if(tr.train_id==-1 || tr.isDeleted == true){
		printf("Train ID not valid!\n");
		return;
	}
	printf("Number of seats available are %d\n", tr.seats_available);	
	printf("Enter the number of seats you want to book, enter -1 to go back!\n");
	printf("Enter choice: ");
	scanf("%d", &seats_needed);
	
	write(sd, &seats_needed, sizeof(seats_needed));
	read(sd, &seats_available, sizeof(seats_available));
	if(seats_needed<=0){
		printf("Seats must be greater then 0\n");
	}else if(seats_needed != -1 && seats_needed <= seats_available){
		read(sd, &booking_id, sizeof(booking_id));
		printf("Booking successful!\n");	
		printf("Booking ID: %d\n", booking_id);	
	}else{
		printf("These many seats not available!\n");
	}
}

void viewBooking(int sd){
	struct booking book;
	read(sd, &book, sizeof(book));
	if(book.isCancled){
		printf("Ticket canceled!\n");
	}
	printf("Booking ID : %d\nTrain ID : %d\nSeats : %d\n", book.book_id, book.train_id, book.seats);
}

void updateBooking(int sd){
	char buffer[50];
	int seats_needed;
	read(sd, buffer, sizeof(buffer));
	printf("%s", buffer);
}

void cancleBooking(int sd){
	bool isValid;
	read(sd, &isValid, sizeof(isValid));
	if(isValid){
		printf("Cancled!\n");
	}else{
		printf("No booking exists!\n");
	}
}

void agent_login(int sd){
	struct database record;
	bool isValid;
		
	while(1){
		printf("Enter username: ");
		scanf("%s", record.username);
		printf("Password: ");
		scanf("%s", record.password);
		
		write(sd, &record, sizeof(record));
		read(sd, &isValid, sizeof(isValid));
		
		if(isValid){
			agent_operations(sd);
			return;
		}
		else
			printf("Invalid Username or Password! Please try again\n");			
	} 
}

void agent_operations(int sd){
	int choice;
	while(1){
		system("clear");
		printf("\n*************************** Welcome Agent ***************************\n\n");
		printf("Select the operation you want to perform:\n1: Book train ticket\n2: View Booking\n3: Cancel Booking\nElse: Go back\n");
		printf("Select the option: ");
		scanf("%d", &choice);
		write(sd, &choice, sizeof(choice));
		switch(choice){
			case 1:bookTicket(sd);
			 	break;
			case 2:viewBooking(sd);
				break;
			case 3:updateBooking(sd);
				break;
			case 4:cancleBooking(sd);
				break;
			default:printf("Invalid Choice!\n");
		}
		printf("Press enter to continue...\n");
		getchar();
		getchar();
	}
}

void admin_login(int sd){
	struct database record;
	bool valid;
	
	while(1){		
		printf("Enter admin username: ");
		scanf("%s", record.username);
		printf("Password: ");
		scanf("%s", record.password);
		
		write(sd, &record, sizeof(record));
		read(sd, &valid, sizeof(valid));
		if(valid){
			admin_operations(sd);
			return;
		}else{
			printf("Invalid Username or Password! Please try again\n");			
		}
	}
}

void admin_operations(int sd){
	int choice;
	
	while(1){
		system("clear");
		printf("\n\n***************************Welcome Admin***************************\n\n");				
		printf("Select the operation you want to perform:\n");
		printf("1: Create User Account\n2: Modify User Account\n3: Delete User Account\n4: Add new train\n5: Modify Train\n6: Delete Train\n7: Search User\n8: Search Train\n9: Display all accounts\nElse: Exit\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));
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
			default:printf("Exiting!\n");
		}
	}
}

void createAccount(int sd){//Done
	struct database record;
	int choice;
	bool isValid;
	
	printf("\nEnter details to create the account:\n");
	printf("\nEnter Username: ");
	scanf("%s", record.username);
	printf("Enter Password: ");
	scanf("%s", record.password);	
	printf("Select Account Type:\n1: Normal-User\n2: Agent-User\n");
	printf("Enter your choice: ");
	scanf("%d", &choice);
	
	if(choice == 1){
		record.account_type = 'n';
		write(sd, &record, sizeof(record));
	}else if(choice == 2){
		record.account_type = 'a';
		write(sd, &record, sizeof(record));
	}else{
		printf("Invalid choice!\n");
		printf("Press enter to go back...\n");
		getchar();
		getchar();
		return;
	}
	
	read(sd, &isValid, sizeof(isValid));
	
	if(isValid){
		printf("Error: Username already exists!\n");
	}else{
		read(sd,&record, sizeof(struct database));
		printf("New account created sucessfully! Account ID: ");
		printf("%d\n", record.account_id);
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
	return;
}

void modifyAccount(int sd){
	int account_id;
	bool isValid;

	printf("Enter details to modify the account:\n");
	printf("Enter account ID: ");
	scanf("%d", &account_id);
	
	write(sd, &account_id, sizeof(account_id));
	read(sd, &isValid, sizeof(isValid));
	
	if(isValid){
		int choice;
		printf("Select the detail you want to modify:\n");
		printf("1: Username\n2: Password\n3: Account Type\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));
		
		switch(choice){
			case 1:modifyUsername(sd);
				break;
			case 2:modifyPassword(sd);
				break;
			case 3:modifyAccountType(sd);
				break;
			default:
				printf("Invalid choice!\n");
		}
	}else{
		printf("Account ID doesn't exists!\n");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void modifyUsername(int sd){
	char username[25];
	bool isValid;
	
	printf("Enter new username: ");
	scanf("%s", username);
	
	write(sd, username, sizeof(username));
	read(sd, &isValid, sizeof(isValid));
	
	if(isValid){
		printf("Username already exists!\n");
	}else{
		printf("Username successfully modified!\n");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void modifyPassword(int sd){
	char password[25];	
	printf("Enter new password: ");
	scanf("%s", password);	
	write(sd, password, sizeof(password));	
	printf("password successfully modified!\n");
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void modifyAccountType(int sd){
	struct database record;
	int choice;
	
	read(sd, &record, sizeof(record));
	if(record.account_type == 'n'){
		printf("Are you sure you want to change account from normal user type to agent type?\n");
	}else{
		printf("Are you sure you want to change account from agent type to normal user type?\n");
	}
	
	while(1){
		printf("1: Yes\n2: No\n:");
		scanf("%d", &choice);
		write(sd, &choice, sizeof(choice));
		if(choice==1){
			printf("Type modified!\n");
			break;
		}else if(choice==2){
			write(sd, &choice, sizeof(choice));
			printf("Cancelling Operation...");
			break;
		}else{	
			printf("Invalid choice! Try again\n");
		}
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void deleteAccount(int sd){//Done
	struct database record;
	bool isValid;
	
	printf("Enter details to delete the account:\n");
	printf("Username: ");
	scanf("%s", record.username);
	write(sd, record.username, sizeof(record.username));
	read(sd, &isValid, sizeof(isValid));
	
	if(isValid){
		printf("Deleting account...\n");
	}else{
		printf("Username doesn't exists!\n");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void addTrain(int sd){
	struct trains tr;
	char buffer[50];
	bool isValid;
	
	printf("Please enter train details: \n");
	printf("Train name: ");
	scanf("%s", tr.name);
	printf("Train number: ");
	scanf("%d", &tr.train_id);
	printf("Total number of seats: ");
	scanf("%d", &tr.seats_available);
	
	write(sd, &tr, sizeof(struct trains));
	read(sd, &isValid, sizeof(isValid));
	if(isValid){
		printf("Train number already exists!\n");
	}else{
		printf("New train added successfully!\n");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void modifyTrain(int sd){
	int train_id;
	bool isValid;

	printf("Enter Train no. of the train to be modified: ");
	scanf("%d", &train_id);
	write(sd, &train_id, sizeof(train_id));
	read(sd, &isValid, sizeof(isValid));
	
	if(isValid){
		int choice;
		printf("Select the detail you want to modify:\n");
		printf("1: Train Name\n2: Number of seats available\n");
		while(1){
			printf("Enter your choice: ");
			scanf("%d", &choice);
			write(sd, &choice, sizeof(choice));
			switch(choice){
				case 1:modifyTrainName(sd);
					return;
				case 2:modifySeats(sd);
					return;
				default:
					printf("Invalid choice! Try again\n");
			}
		}
	}else{
		printf("Train number doesn't exists!\n");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}



void modifyTrainName(int sd){
	char name[25];
	printf("Enter new train name: ");
	scanf("%s", name);
	write(sd, name, sizeof(name));
	printf("Train Name updated successfully!\n");
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void modifySeats(int sd){
	int seats;
	printf("Enter the updated number of seats available: ");
	scanf("%d", &seats);
	write(sd, &seats, sizeof(seats));
	printf("Number of seats updated successfully!\n");
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void deleteTrain(int sd){
	bool isValid;
	int choice, train_id;
	
	printf("Enter train number of the train to be deleted:\n");
	scanf("%d", &train_id);
	write(sd, &train_id, sizeof(train_id));
	read(sd, &isValid, sizeof(isValid));
	if(!isValid)
		printf("Train doesn't exists!");
	else{
		write(sd, &choice, sizeof(choice));
		printf("Train deleted successfully!");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}                  

void searchUser(int sd){//Done
	struct database record;
	printf("Enter username to search: ");
	scanf("%s", record.username);	
	write(sd, record.username, sizeof(record.username));
	read(sd, &record, sizeof(record));
	if(record.account_id == -1 || record.isDeleted==true){
		printf("User not found!\n");
	}else{
		printf("Account ID: %d\n", record.account_id);
		printf("Username: %s\n", record.username);
		printf("Account Type: %s\n", record.account_type=='a'?"Agent User":"Normal User");
		printf("Is deleted: %s\n", record.isDeleted?
		"Yes":"No");
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void searchTrain(int sd){//Done
	struct trains tr;
	int train_id;
	
	printf("Enter train number to search: ");
	scanf("%d", &train_id);
	
	write(sd, &train_id, sizeof(train_id));
	read(sd, &tr, sizeof(tr));
	if(tr.train_id == -1 || tr.isDeleted==true){
		printf("Train not found!\n");
	}else{
		printf("Train Number: %d\n", tr.train_id);
		printf("Train Name: %s\n", tr.name);
		printf("Seats Available: %d\n", tr.seats_available);
	}
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}

void displayAccounts(int sd){
	struct database record;
	printf("\nAll account details: \n");
	read(sd, &record, sizeof(record));
	while(record.account_id!=-1){
		read(sd, &record, sizeof(record));
		if(record.account_id==-1)
			break;
		if(record.isDeleted)
			continue;
		printf("\nAccount ID: %d\n", record.account_id);
		printf("Username: %s\n", record.username);
		printf("Password: %s\n", record.password);
		printf("Account Type: %s\n", record.account_type=='a'?"Agent User":"Normal User");
	}
	printf("_________________________________________\n");
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}
/*
void displayTrains(int sd){
	struct trains tr;
	printf("\nAll train details: \n");
	read(sd, &tr, sizeof(tr));
	while(tr.train_id!=-1){
		read(sd, &tr, sizeof(tr));
		if(tr.train_id==-1)
			break;
		if(tr.isDeleted)
			continue;
		printf("\nTrain Number: %d\n", tr.train_id);
		printf("Train Name: %s\n", tr.name);
		printf("Seats Available: %d\n", tr.seats_available);
	}
	printf("_________________________________________\n");
	printf("Press enter to continue...\n");
	getchar();
	getchar();
}
*/
