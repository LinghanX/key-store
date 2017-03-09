struct request {
	// 0 find; 1 get; 2 set; 
	// 3 set success; 4 get success
	// 5 get no result;
	// 6 fail
	int flag; 
	size_t ksize; 
    size_t vsize;
};
#define RSIZE sizeof(struct request)
#define FIND 0
#define GET 1
#define SET 2
