#include "server.hpp"

Server::Server(const std::string& host_, int port_, orderBook& engine, orderProducer& orderProducer_):
    host(host_),
    port(port_),
    book(engine),
    kafkaProducer(orderProducer_),
    running(false)
    {
        createSocket();
    }

void Server::startServer(){

    struct sockaddr_in address;

    int opt = 1;

    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    }

    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT");
        throw std::runtime_error("Failed to set SO_REUSEPORT");
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, host.c_str(), &address.sin_addr);

    if(bind(socketFD, (sockaddr*)&address, sizeof(address)) < 0){ 
        throw std::runtime_error("Socket bind failed");
    }

    if(listen(socketFD, 1000) < 0){
        throw std::runtime_error("Failed to listen");
    }
    setNonBlocking(socketFD); 
    running = true;
    setUpKqueue();
    broadcaster =  std::thread(&Server::broadcast, this);
    listener = std::thread(&Server::Listen, this); // listener thread 
    for(int i = 0; i < threadSize; i++){
        clientThreads[i] = std::thread(&Server::handleEvent, this ,i);
    }
}

void Server::createSocket(){
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // creates the socket 
    if(socketFD < 0){
        throw std::runtime_error("Socket failed");
    }
}

void Server::Listen(){
    int workerN = 0;
    while(running){
        int clientFD = accept(socketFD, nullptr, nullptr);
        if(clientFD < 0){ // error 
            if (errno == EMFILE || errno == ENFILE) {
                std::cout << "ERROR: File descriptor limit reached" << std::endl;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No new connection
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                  std::cout << "Afail" << std::endl;
                continue;
            }
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientFD);
        }
        setNonBlocking(clientFD);
        fillKQueue(workerN,clientFD);
        workerN++;
        std::lock_guard<std::mutex> lock(clientsMutex);       
        if(workerN == threadSize) workerN = 0;
    }
}   

void Server::handleEvent(int clientN){
    int currentQueue = workerQueues[clientN];
    while(running){
        struct timespec timeout = {0, 100 * 1000 * 1000}; // 100 ms
        int numEvents = kevent(currentQueue, NULL, 0 , events[clientN] , maxSize , &timeout);
        if(numEvents < 0 ){
            perror("noEvent");
            continue;
        }
        for (int i = 0; i < numEvents; i++) {
            int fd = events[clientN][i].ident;
            if (fd == socketFD) {
                continue; // new connection -> do nothing
            }
            else if(events[clientN][i].filter == EVFILT_READ){
                char buffer[1024];
                ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
                if (bytesRead == 0) { // disconnect 
                    {
                        std::lock_guard<std::mutex> lock(clientsMutex);
                        clients.erase(std::remove(clients.begin(), clients.end(), fd),clients.end());
                    }
                    struct kevent delEv;
                    EV_SET(&delEv, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    kevent(currentQueue, &delEv, 1, NULL, 0, NULL);
                    close(fd);
                } 
                else if (bytesRead > 0){
                    std::string response = processEvent(std::string(buffer, bytesRead));
                    send(fd, response.c_str(), response.size(), 0);
                }
                else{
                    if(errno == EAGAIN || errno == EWOULDBLOCK){
                        continue;
                    }
                }
            }
        }
    }
}

std::string Server::processEvent(const std::string& message){
    messageParser parser;
    if(!parser.validateFormat(message)){
        return "Incorrect Format \n";
    }
    parser.evaluate(message);
    orderType t;
    if(parser.getType() == "BUY"){
        t = orderType::BUY;
    }
    else{
        t = orderType::SELL;
    }
    if(!book.validPrice(parser.getPrice())){
        return "Invalid price \n";
    }
    {
        std::lock_guard<std::mutex> lock(bookMutex);
        book.addOrder(t, parser.getPrice(), parser.getVolume());
    }
    std::string response = "Order fufilled \n";
    return response;
}

void Server::setUpKqueue(){
    for(int i =0 ; i < threadSize; i++){
        workerQueues[i] = kqueue(); // create a kqueue for each thread
        if(workerQueues[i] < 0){
            throw std::runtime_error("Failed to create worker kqueue");
        }
    } 
}

void Server::closeServer(){
    running = false;
    shutdown(socketFD, SHUT_RDWR);
    close(socketFD);
    socketFD = -1;
    if(listener.joinable()){
        listener.join();
    }
    if(broadcaster.joinable()){
        broadcaster.join();
    }
    for(int i = 0; i < threadSize; i++){
        if(clientThreads[i].joinable()) clientThreads[i].join();
    }
    for(int i = 0; i < threadSize; i++){
        close(workerQueues[i]);
    }
}

void Server::broadcast(){
    int counter = 0;
    while (running) {
        if(book.getRecentChange()){
            {
            std::lock_guard<std::mutex> lock(clientsMutex);
            kafkaProducer.produce(book.toJson());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        else{
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Server::fillKQueue(int worker, int workerFD){
    struct kevent newEv;
    EV_SET(&newEv, workerFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(workerQueues[worker], &newEv, 1, NULL, 0, NULL) == -1) {
        throw std::runtime_error("Failed to add client FD to worker kqueue");
    }
}

void Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
