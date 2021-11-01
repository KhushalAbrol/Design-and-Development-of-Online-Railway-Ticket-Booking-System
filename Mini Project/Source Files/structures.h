struct booking{
	bool isCancled;
	int account_id;
	int train_id;
	int seats;
	int book_id;
};

struct database{
	char username[25];
	char password[25];
	int account_id;
	struct booking book;
	char account_type;//n -> Normal user, a -> Agent user
	bool isDeleted;
};

struct trains{
	int train_id;
	int seats_available;
	char name[25];
	bool isDeleted;
};
