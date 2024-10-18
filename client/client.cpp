#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>


using namespace std;

const int BufferSize = 4096;
const int NETWORK_WAITING_QUEUE = 100;

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

// Function Declarations
void *process_request(void *client_socket);
void *handle_peer_request(void *client_listen_port);
void handle_error(int exp, const char *msg);
vector<string> splitString(const string& command);
string CalculateSHA1(string file_buffer_string);
string getFileName(string path);
void send_file(int client_sd, string f_path);
void extract_port(vector<string> &tracker_info, string path);
bool receive_file(string fil_name, string d_path, int client_socket);
string piece_select(int chunk_number);



// Global Data Structures
unordered_map<string, string> my_file_path;                   // filename - its path
unordered_map<int, vector<string>> chunk_details;          // chunk number - user IDs which have the chunk
unordered_map<string, vector<string>> download_info;       // User ID - List of downloaded files



int main(int argc, const char *arguments[])
{
    int my_client_socket;
    // Create a socket for the client to communicate with the server
    handle_error(my_client_socket = socket(AF_INET, SOCK_STREAM, 0), "Failed to create socket");

    // Extract client's IP and port from the command line arguments
    string my_client_ip_port = arguments[1];
    int my_pos = my_client_ip_port.find(":");
    string my_client_port = my_client_ip_port.substr(my_pos + 1);
    int my_listen_port = atoi(my_client_port.data());

    // Create a thread to handle peer requests
    pthread_t my_thread;
    pthread_create(&my_thread, NULL, handle_peer_request, &my_listen_port);

    // Extract tracker information from arguments
    vector<string> my_tracker_info;
    string my_tracker_info_path = arguments[2];
    extract_port(my_tracker_info, my_tracker_info_path);

    // Extract the port of the first tracker
    string my_first_tracker = my_tracker_info[0];
    int my_pos1 = my_first_tracker.find(":");
    string my_server_port = my_first_tracker.substr(my_pos1 + 1);
    int my_port_no = atoi(my_server_port.data());

    // Create a server address structure to connect to the tracker
    struct sockaddr_in my_server_address;
    my_server_address.sin_family = AF_INET;
    my_server_address.sin_port = htons(my_port_no);
    my_server_address.sin_addr.s_addr = INADDR_ANY;

    // Attempt to connect to the tracker
    int my_connection_status = connect(my_client_socket, (struct sockaddr *)&my_server_address, sizeof(my_server_address));

    // Check if the connection to the tracker was successful
    if (my_connection_status == -1)
    {
        printf("There was an error making a connection to the remote socket \n\n");
        exit(0);
    }

    char my_server_response[2000];
    string my_client_message;
    unordered_map<string, vector<string>> my_download_info;
    
    // Main loop for user interaction
    for (;;)
    {
        cout << "> ";

        getline(cin, my_client_message);
        vector<string> my_client_command = splitString(my_client_message);
        bool my_flag = 1;

        // If the user is logging in, append the client port to the message
        if (my_client_command[0] == "login")
            my_client_message += (" " + my_client_port);
        // If uploading a file, modify the message and store file information
        else if (my_client_command[0] == "upload_file" && my_client_command.size() == 3)
        {
            string my_file_name = getFileName(my_client_command[1]);
            my_client_message = "";
            my_client_message = my_client_command[0] + " " + my_file_name + " " + my_client_command[2];
            my_client_message += (" " + CalculateSHA1(my_client_command[1]));
            if (CalculateSHA1(my_client_command[1]) == "-1")
                my_flag = 0;
            else
                my_file_path[my_file_name] = my_client_command[1];
        }

        if (!my_flag)
        {
            cout << "Invalid file path. \n\n";
            continue;
        }

        // Send the client message to the tracker
        send(my_client_socket, my_client_message.data(), 2000, 0);
        bzero(my_server_response, 2000);
        recv(my_client_socket, my_server_response, 2000, 0);

        // Process the server's response based on the command and conditions
        string serverResponse = string(my_server_response);
        if (my_client_command[0] == "download_file" &&
            serverResponse != "Enter valid command" &&
            serverResponse != "Please login first" &&
            serverResponse != "Group does not exist" &&
            serverResponse != "You are not a member of this group" &&
            serverResponse != "No such group exists")
        {
            if (!strlen(my_server_response))
                cout << "No clients available to share the file. \n\n";
            else
            {
                vector<string> my_ports_to_connect = splitString(my_server_response);
                if (my_ports_to_connect[0] != "This" || my_ports_to_connect[0] != "Group")
                {
                    for (auto my_a : my_ports_to_connect)
                    {
                        // Create a socket to connect to a peer
                        int my_client_server_socket;
                        my_client_server_socket = socket(AF_INET, SOCK_STREAM, 0);

                        int my_port_no = atoi(my_a.data());

                        // Create a server address structure to connect to the peer
                        struct sockaddr_in my_server_address;
                        my_server_address.sin_family = AF_INET;
                        my_server_address.sin_port = htons(my_port_no);
                        my_server_address.sin_addr.s_addr = INADDR_ANY;

                        bool my_flag1 = 0;

                        // Connect to the peer server
                        int my_connected_peer_server = connect(my_client_server_socket, (struct sockaddr *)&my_server_address, sizeof(my_server_address));
                        if (my_connected_peer_server != -1)
                        {
                            // Send the file name to the peer and receive the file
                            send(my_client_server_socket, my_client_command[2].data(), 2000, 0);
                            if (receive_file(my_client_command[2], my_client_command[3], my_client_server_socket))
                            {
                                my_download_info[my_client_command[1]].push_back(my_client_command[2]);
                                my_file_path[my_client_command[2]] = my_client_command[3] + "/" + my_client_command[2];
                                string my_success = "downloaded " + my_client_command[1] + " " + my_client_command[2];
                                send(my_client_socket, my_success.data(), 2000, 0);
                                bzero(my_server_response, 2000);
                                recv(my_client_socket, my_server_response, 2000, 0);
                            }
                            else
                                cout << "There was some error in downloading file \n\n";
                            my_flag1 = 1;
                            close(my_client_server_socket);
                        }
                        if (my_flag1)
                            break;
                    }
                }
                else
                    cout << my_server_response << "\n\n";
            }
        }
        else if (my_client_command[0] == "show_downloads" && (string(my_server_response) != "Please login first"))
        {
            if (!my_download_info.size())
                cout << "No download info currently! \n\n";
            else
            {
                for (auto my_a : my_download_info)
                {
                    cout << "[D] " << my_a.first << endl;
                    cout << "[C] " << my_a.first;
                    for (auto my_b : my_a.second)
                        cout << " " << my_b;
                    cout << "\n\n";
                }
            }
        }
        else
            cout << my_server_response << "\n\n";
        if (strcmp(my_client_message.data(), "quit") == 0)
            break;
    }

    close(my_client_socket);

    return 0;
}



void *handle_peer_request(void *clientPort)
{
    int port = *((int *)clientPort);

    int serverSock, clientSock, size;
    SA_IN serverAddr, clientAddr;

    handle_error((serverSock = socket(AF_INET, SOCK_STREAM, 0)),"Socket creation failed");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    handle_error((bind(serverSock, (SA *)&serverAddr, sizeof(serverAddr))),"Bind operation failed!");
    handle_error((listen(serverSock, NETWORK_WAITING_QUEUE)),"Listen operation failed!");
    pthread_t threadId[100];
    int index = 0;
     for(;;)
    {

        size = sizeof(SA_IN);
        clientSock = accept(serverSock, (SA *)&clientAddr, (socklen_t *)&size);


        pthread_create(&threadId[index++], NULL, process_request, &clientSock);
    }
    for (int counter = 0; counter < index; counter++)
    {
        pthread_join(threadId[counter], NULL);
    }
}

void *process_request(void *p_client_socket) {
    int client_socket = *((int *)p_client_socket);

    string user_id;

    char client_message[2000];
    memset(client_message, 0, sizeof(client_message));

    // Receive the client request.
    ssize_t bytes_received = recv(client_socket, client_message, sizeof(client_message), 0);

    if (bytes_received < 0) {
        // Handle the error condition (e.g., logging, sending an error response).
        cerr << "Error receiving data from the client." << endl;
    } else {
        // Handle the received client message (e.g., extract user_id, process the request).
        user_id = client_message;

        if (my_file_path.find(user_id) != my_file_path.end()) {
            send_file(client_socket, my_file_path[user_id]);

        } else {
            // Handle the case when the requested user_id is not found (e.g., send an error response).
            cerr << "User ID not found." << endl;
        }
    }

    // Close the client socket.
    close(client_socket);

    return NULL;
}

void handle_error(int expression, const char *message)
{
    if (expression == -1)
    {
        // Log the error to a file
        FILE *errorLog = fopen("error.log", "a");
        if (errorLog != NULL)
        {
            fprintf(errorLog, "Error: %s\n", message);
            fclose(errorLog);
        }

        perror(message);
        exit(1);
    }
}

vector<string> splitString(const string& command) {
    vector<string> tokens;
    istringstream tokenizer(command);
    string token;

    while (tokenizer >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

string CalculateSHA1(string file_buffer_string)
{
    string hash;
    unsigned char md[20];
    if (!SHA1(reinterpret_cast<const unsigned char *>(&file_buffer_string[0]), file_buffer_string.length(), md))
    {
        printf("Error in hashing\n");
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            char buf[3];
            sprintf(buf, "%02x", md[i] & 0xff);
            hash += string(buf);
        }
    }

    string sha;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sha += hash[i];

    return sha;
}

string getFileName(string path)
{
    string file_name;
    size_t found;
    found = path.find_last_of('/');
    if (found == string::npos)
        file_name = path;
    else
        file_name = path.substr(found + 1);
    return file_name;
}

void send_file(int clientSocket, string filePath) {
    int fileDescriptor = open(filePath.c_str(), O_RDONLY);

    char dataBuffer[BufferSize];
    bzero(dataBuffer, BufferSize);

    for (;;) {
        int bytesRead = read(fileDescriptor, dataBuffer, BufferSize);
        if (bytesRead <= 0)
            break;
        int bytesSent = send(clientSocket, dataBuffer, bytesRead, 0);
        bzero(dataBuffer, BufferSize);

        if (bytesSent <= 0)
            break;
    }
}


void extract_port(vector<string> &tracker_info, string path)
{
    // Open the file at the specified path for reading.
    fstream file;
    file.open(path.c_str(), ios::in);

    string line;
    while (getline(file, line))
    {
        // Read each line from the file and append it to the tracker_info vector.
        tracker_info.push_back(line);
    }

    // Close the file after reading.
    file.close();
}

string piece_select(int chunk_number) {
    string u_id;
    if (chunk_details.find(chunk_number) != chunk_details.end()) {
        vector<string>& user_ids = chunk_details[chunk_number];
        if (!user_ids.empty()) {
            sort(user_ids.begin(), user_ids.end());
            u_id = user_ids[user_ids.size() / 2];
        }
    }
    return u_id;
}

bool receive_file(string file_name, string destination_path, int client_socket) {
    // Construct the full file path at the destination
    string file_path = destination_path + "/" + file_name;

    // Open the file for writing
    int destination_file_descriptor = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if (destination_file_descriptor < 0) {
        cout << "Error in opening file or directory doesn't exist." << endl;
        return false;
    }

    char buffer[BufferSize];
    bzero(buffer, BufferSize);
    cout << "Downloading file..." << endl;
    int total_bytes_received = 0;

    for (;;) {
        int bytes_received = recv(client_socket, buffer, BufferSize, 0);
        if (bytes_received < 0) {
            cout << "Error in receiving file." << endl;
            break;
        } else if (bytes_received == 0) {
            // End of file
            break;
        }

        int bytes_written = write(destination_file_descriptor, buffer, bytes_received);
        if (bytes_written < 0) {
            cout << "Error in writing file." << endl;
            break;
        }

        total_bytes_received += bytes_received;
        bzero(buffer, BufferSize);
    }

    cout << "Downloaded file successfully! Total bytes received = " << total_bytes_received << "\n\n";

    // Close the destination file
    close(destination_file_descriptor);

    return true;
}



