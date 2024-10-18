#define Request_Queue_Capacity 1 // Request_Queue_Capacity represents the maximum number of client requests that can be concurrently accepted by the server.

typedef struct sockaddr_in Sa_In;
typedef struct sockaddr Sa;

#include <stdbool.h>
#include <bits/stdc++.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <limits.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;



string Extract_file_name(const string &path);
bool get_port(vector<string> &tracker_info, const string &path);
vector<string> splitString(const string command, char c);
void *process_request(void *client_socket);
void handle_error(int condition, const char *message);                            




unordered_map<string, bool> loggedInByUserId;                                      // Mapping from userid to true/false for logged-in status
unordered_map<string, vector<string>> requestsByGroupId;                           // Mapping from groupid to user ids of people who made requests
unordered_map<string, vector<pair<string, string>>> filesByGroupId;                // Mapping from groupid to filename and its SHA
unordered_map<string, vector<string>> membersByGroupId;                            // Mapping from groupid to user ids of the members
unordered_map<string, string> passwordByUserId;                                    // Mapping from userid to password
unordered_map<string, unordered_map<string, vector<string>>> fileInfoByGroupId;    // Mapping from groupid to filename and the user ids which have this file
unordered_map<string, string> ipPortByUserId;                                      // Mapping from userid to IP and port



int main(int argc, const char *argv[])
{
    // Check if the required command-line argument is provided.
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <tracker_info_path>" << endl;
        exit(1);
    }

    Sa_In serverAddress, clientAddress;
    vector<string> trackerInfo;
    int listenerSocket, clientSocket, addressSize;

    // Create a socket to listen for incoming connections.
    handle_error((listenerSocket = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create server socket");

    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;

    string trackerInfoPath = argv[1];

    // Extract and parse port information from the tracker_info_path.
    get_port(trackerInfo, trackerInfoPath);

    string trackerOne = trackerInfo[0];
    int position = trackerOne.find(":");
    string portString = trackerOne.substr(position + 1);
    int portNumber = atoi(portString.data());

    serverAddress.sin_port = htons(portNumber);

    // Bind the server socket to the specified address and port.
    handle_error((bind(listenerSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))),"Bind failed");

    // Start listening for incoming client connections.
    handle_error((listen(listenerSocket, Request_Queue_Capacity)),"Listen failed"); 

    for (;;) {
        cout << "Listening for incoming connections..." << endl;
        addressSize = sizeof(struct sockaddr_in);

        // Accept a new client connection.
        handle_error((clientSocket = accept(listenerSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addressSize)),"Connection acceptance failed");
        cout << "Connected!" << endl;

        pthread_t thread;
        // Create a new thread to process the client request.
        pthread_create(&thread, NULL, process_request, &clientSocket);
    }

    // Close the listener socket and exit.
    close(listenerSocket);
    return 0;
}



bool validateCommand(string first, vector<string> command)
{
    int commandSize = command.size();

    switch (commandSize) {
        case 4:
            return (first == "download_file" || first == "login" || first == "upload_file");
        case 3:
            return (first == "create_user" || first == "accept_request" || first == "stop_share");
        case 2:
            return (first == "create_group" || first == "join_group" || first == "leave_group" || first == "list_requests" || first == "list_files");
        case 1:
            return (first == "list_groups" || first == "logout" || first == "show_downloads");
        default:
            return false;
    }

}

void *process_request(void *p_client_socket)
{
    int client_socket = *((int *)p_client_socket);

    string current_user_id = "";

    string server_response;
    char client_message[2000];

    for (;;)
    {
        bzero(client_message, 2000);
        if (recv(client_socket, client_message, 2000, 0) <= 0)
            break;

        vector<string> command = splitString(client_message, ' ');

        // Check if the command is "downloaded"
        if (command[0] == "downloaded")
        {
            // Update fileInfoByGroupId with the user's information
            fileInfoByGroupId[command[1]][command[2]].push_back(current_user_id);
            server_response = "Okay! Acknowledged that now you can also share";
            send(client_socket, server_response.data(), 2000, 0);
            continue;
        }
        cout << "Client: " << client_message << endl;

        // Check if the command is "quit"
        if (strcmp(client_message, "quit") == 0)
        {
            server_response = "Goodbye!";
            send(client_socket, server_response.data(), 2000, 0);
            close(client_socket);  // Close the client connection
            // close(server_socket);  // Close the server socket
            exit(0);  // Exit the server
        }
        else
        {
            // Check if the command is valid
            if (!validateCommand(command[0], command))
            {
                server_response = "Please provide a valid command";
                send(client_socket, server_response.data(), 2000, 0);
                continue;
            }

            bool flag = 0;
            if (command[0] != "login" && command[0] != "create_user")
            {
                if (current_user_id.size() == 0 || !loggedInByUserId[current_user_id])
                {
                    server_response = "Kindly log in before proceeding";
                    send(client_socket, server_response.data(), 2000, 0);
                    flag = 1;
                }
            }
            if (flag)
                continue;

            // Check if the command is "create_user"
            if (command[0] == "create_user")
            {
                if (passwordByUserId.count(command[1]))
                    server_response = "User is already registered!";
                else
                {
                    passwordByUserId[command[1]] = command[2];
                    server_response = "User created!";
                }
            }

            // Check if the command is "create_group"
            else if (command[0] == "create_group")
            {
                if (membersByGroupId.find(command[1]) != membersByGroupId.end())
                    server_response = "Group already exists!";
                else
                {
                    membersByGroupId[command[1]].push_back(current_user_id);
                    server_response = "Group creation successful.";
                }
            }

            // Check if the command is "login"
            else if (command[0] == "login")
            {
                if (!passwordByUserId.count(command[1]))
                    server_response = "No such user found.";
                else
                {
                    if (loggedInByUserId[current_user_id])
                        server_response = "You are already connected.";
                    else
                    {
                        server_response  = "Login was successful.";
                        current_user_id = command[1];
                        loggedInByUserId[current_user_id] = 1;
                        ipPortByUserId[current_user_id] = command[command.size() - 1];
                    }
                }
            }

            // Check if the command is "leave_group"
            else if (command[0] == "leave_group")
            {
                string group_id = command[1];
                if (!membersByGroupId.count(group_id))
                    server_response = "The group is not found.";
                else
                {
                    auto group_members = membersByGroupId[group_id];
                    auto it = find(group_members.begin(), group_members.end(), current_user_id);
                    if (it == group_members.end())
                        server_response = "You are not a member of this group.";
                    else
                    {
                        if (membersByGroupId[group_id].size() == 1)
                        {
                            membersByGroupId.erase(group_id);
                            requestsByGroupId.erase(group_id);
                            filesByGroupId.erase(group_id);
                            fileInfoByGroupId.erase(group_id);
                            server_response = "The group has been deleted because you were the only member.";
                        }
                        else
                        {
                            vector<string> temp;
                            for (auto member : membersByGroupId[group_id])
                                if (member != current_user_id)
                                    temp.push_back(member);

                            membersByGroupId[group_id] = temp;
                            auto file_owners = fileInfoByGroupId[group_id];
                            for (auto owner : file_owners)
                            {
                                vector<string> temp;
                                for (auto user : owner.second)
                                    if (user != current_user_id)
                                        temp.push_back(user);
                                fileInfoByGroupId[group_id][owner.first] = temp;
                            }
                            server_response = "You have exited the group.";
                        }
                    }
                }
            }
            // Check if the command is "join_group"
            else if (command[0] == "join_group")
            {
                if (!membersByGroupId.count(command[1]))
                    server_response = "No such group exists.";
                else
                {
                    auto group_members = membersByGroupId[command[1]];
                    if (find(group_members.begin(), group_members.end(), current_user_id) != group_members.end())
                        server_response = "You are already part of the group.";
                    else
                    {
                        vector<string> current_requests = requestsByGroupId[command[1]];
                        auto it = find(current_requests.begin(), current_requests.end(), current_user_id);
                        if (it != current_requests.end())
                            server_response = "Your join request has been sent.";
                        else
                        {
                            requestsByGroupId[command[1]].push_back(current_user_id);
                            server_response = "Your join request has been successfully submitted.";
                        }
                    }
                }
            }

            // Check if the command is "list_requests"
            else if (command[0] == "list_requests")
            {
                if (!membersByGroupId.count(command[1]))
                    server_response = "The group does not exist.";
                else
                {
                    auto group_members = membersByGroupId[command[1]];
                    if (group_members[0] != current_user_id)
                        server_response = "You do not have administrative privileges for this group.";
                    else
                    {
                        if (!requestsByGroupId.count(command[1]) || requestsByGroupId[command[1]].size() == 0)
                            server_response = "There are no pending join requests at the moment.";
                        else
                        {
                            server_response = "";
                            for (auto request : requestsByGroupId[command[1]])
                                server_response += (request + " ");
                        }
                    }
                }
            }

            // Check if the command is "list_groups"
            else if (command[0] == "list_groups")
            {
                if (membersByGroupId.size() == 0)
                    server_response = "There are no groups available at the moment.";
                else
                {
                    server_response = "";
                    for (auto group : membersByGroupId)
                        server_response += (group.first + " ");
                }
            }

            // Check if the command is "accept_request"
            else if (command[0] == "accept_request")
            {
                if (!membersByGroupId.count(command[1]))
                    server_response = "The group is not found.";
                else
                {
                    auto group_members = membersByGroupId[command[1]];
                    if (group_members[0] != current_user_id)
                        server_response = "You do not have administrative privileges for this group.";
                    else
                    {
                        if (!requestsByGroupId.count(command[1]))
                            server_response = "There are no pending join requests at the moment.";
                        else
                        {
                            auto it = find(requestsByGroupId[command[1]].begin(), requestsByGroupId[command[1]].end(), command[2]);
                            if (it == requestsByGroupId[command[1]].end())
                                server_response = "There is no join request from this user_id for the group.";
                            else
                            {
                                membersByGroupId[command[1]].push_back(command[2]);
                                requestsByGroupId[command[1]].erase(it);
                                server_response = "User has been approved.";
                            }
                        }
                    }
                }
            }

            // Check if the command is "list_files"
            else if (command[0] == "list_files")
            {
                if (!membersByGroupId.count(command[1]))
                    server_response = "Group does not exist";
                else
                {
                    auto group_members = membersByGroupId[command[1]];
                    auto it = find(group_members.begin(), group_members.end(), current_user_id);
                    if (it == group_members.end())
                        server_response = "You are not a member of this group";

                    else
                    {
                        server_response = "";
                        if (!filesByGroupId.count(command[1]) || filesByGroupId[command[1]].size() == 0)
                            server_response = "There are no files in the group currently.";
                        else
                            for (auto group_file : filesByGroupId[command[1]])
                                server_response += (group_file.first + " ");
                    }
                }
            }

            // Check if the command is "download_file"
            else if (command[0] == "download_file")
            {
                string group_id = command[1], file_name = command[2];
                if (!membersByGroupId.count(group_id))
                    server_response = "Group does not exist";
                else
                {
                    auto group_members = membersByGroupId[group_id];
                    auto it = find(group_members.begin(), group_members.end(), current_user_id);
                    if(it == group_members.end())
                        server_response = "You are not a member of this group.";
                    else
                    {
                        bool flag = 0;
                        for (auto group_file : filesByGroupId[group_id])
                            if (group_file.first == file_name)
                                flag = 1;
                        if (!flag)
                            server_response = "The file is not found in the group.";
                        else
                        {
                            server_response = "";
                            vector<string> user_list = fileInfoByGroupId[group_id][file_name];
                            for (auto u_id : user_list)
                            {
                                if (loggedInByUserId[u_id]) // give the port of only logged in users
                                    server_response += (ipPortByUserId[u_id] + " ");
                            }
                        }
                    }
                }
            }

            // Check if the command is "upload_file"
            else if (command[0] == "upload_file")
            {
                string group_id = command[2];

                if (!membersByGroupId.count(group_id))
                    server_response = "The group is not available.";
                else
                {
                    auto group_members = membersByGroupId[group_id];
                    auto it = find(group_members.begin(), group_members.end(), current_user_id);
                    if (it == group_members.end())
                        server_response = "You are not a member of this group.";
                    else
                    {
                        string file_name = Extract_file_name(command[1]);
                        string sha = command[3];
                        filesByGroupId[group_id].push_back(make_pair(file_name, sha));
                        fileInfoByGroupId[group_id][file_name].push_back(current_user_id);
                        server_response = "File upload was successful.";
                    }
                }
            }

            // Check if the command is "logout"
            else if (command[0] == "logout")
            {
                loggedInByUserId[current_user_id] = 0;
                ipPortByUserId.erase(current_user_id);
                current_user_id = "";
                server_response = "Logout successful.";
            }

            // Check if the command is "show_downloads"
            else if (command[0] == "show_downloads")
            {
                server_response = "Displaying downloads:";
            }

            // If none of the above conditions are met, it's assumed to be "stop_share"
            else
            {
                string group_id = command[1], file_name = command[2];

                if (!membersByGroupId.count(group_id))
                    server_response = "Group does not exist";
                else
                {
                    vector<pair<string, string>> group_files_sha = filesByGroupId[group_id]; // filenames and their SHA
                    vector<pair<string, string>> temp;

                    for (auto it = group_files_sha.begin(); it != group_files_sha.end(); ) {
                        if (it->first != file_name) {
                            temp.push_back(*it);
                            it = group_files_sha.erase(it); // Erase and move to the next element
                        } else {
                            ++it; // Move to the next element without erasing
                        }
                    }

                    if (temp == group_files_sha)
                        server_response = "The file is not found in the group.";
                    else
                    {
                        vector<string> user_list = fileInfoByGroupId[group_id][file_name];
                        vector<string> temp1;
                        for (auto user : user_list)
                            if (user != current_user_id)
                                temp1.push_back(user);

                        if (temp1 == user_list)
                            server_response = "You do not possess this file.";
                        else
                        {
                            if (user_list.size() == 1)
                            {
                                if (temp.size() == 1) // only one file was there in the group
                                    filesByGroupId.erase(group_id);
                                else
                                    filesByGroupId[group_id] = temp; // removing the file from the group_files if only one user has it
                                fileInfoByGroupId[group_id].erase(file_name);
                            }
                            else
                                fileInfoByGroupId[group_id][file_name] = temp1;
                            server_response =  "Sharing has been terminated for " + file_name;
                        }
                    }
                }
            }
        }
        send(client_socket, server_response.data(), 2000, 0);
    }
    close(client_socket);
    return NULL;
}



void handle_error(int condition, const char *message) {
    if (condition == -1) {
        perror(message);
        exit(1);
    }
}

// Function to get the file name from a given path
string Extract_file_name(const string &path) {
    // Find the last occurrence of '/' character
    size_t lastSlash = path.find_last_of('/');

    // If '/' is not found, the whole path is the file name
    if (lastSlash == string::npos) {
        return path;
    } else {
        // Extract and return the file name (substring after the last '/')
        return path.substr(lastSlash + 1);
    }
}

// Function to read port information from a file and store it in a vector
bool get_port(vector<string> &tracker_info, const string &path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << path << endl;
        return false;  // Return false to indicate an error
    }

    string line;
    while (getline(file, line)) {
        tracker_info.push_back(line);
    }

    file.close();
    return true;  // Return true to indicate success
}

// Split a string into tokens using a delimiter character
vector<string> splitString(const string command, char c) {
    vector<string> tokens; // Resulting tokens
    string token;         // Temporary token
    
    for (char character : command) {
        if (character == c) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += character;
        }
    }
    
    if (!token.empty()) {
        tokens.push_back(token);
    }
    
    return tokens;
}