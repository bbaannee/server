#include "WebServer.h"

int main() {
    // Creating instance of server on port 8080
    WebServer app(8080);
    
    // starting server
    app.start();

    return 0;
}