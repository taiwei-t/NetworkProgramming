
int read_timeout(int fd, unsigned int wait_seconds);

int write_timeout(int fd, unsigned int wait_seconds);

int accept_timeout(int fd, struct sockaddr_in* addr, unsigned int wait_seconds);

void activate_nonblock(int fd);

void deactivate_nonblock(int fd);

int connect_timeout(int fd, struct sockaddr_in* addr, unsigned int wait_seconds);

