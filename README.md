# P2P-File-Sharing


---

# Distributed File Sharing System

## Description
This project implements a distributed file sharing system where users can share and download files from a group they belong to. The system is based on socket programming and involves multiple peers sharing files concurrently. The system includes synchronized trackers, parallel downloading, and custom piece selection algorithms.

## Features
- **User Registration and Login**: Users can create an account and log in to access the file sharing system.
- **Group Management**: Users can create, join, and manage groups. The group creator becomes the group owner.
- **File Sharing**: Users can share files within a group. Shared files are listed and can be downloaded by other group members.
- **Parallel File Downloading**: Files are downloaded in parallel from multiple peers using a custom piece selection algorithm.
- **File Integrity**: SHA1 hashing is used to ensure file integrity during file sharing and downloading.
- **Tracker Synchronization**: Multiple trackers are synchronized to maintain the state of shared files and peer information.
- **Concurrent File Downloads**: Users can download multiple files concurrently, with ongoing downloads displayed.

## Commands

### Tracker
- **Run Tracker**:  
  ```bash
  ./tracker tracker_info.txt tracker_no
  ```
  The `tracker_info.txt` contains IP and port details of all the trackers.
  
- **Stop Tracker**:  
  ```bash
  quit
  ```

### Client
- **Run Client**:  
  ```bash
  ./client <IP>:<PORT> tracker_info.txt
  ```
  The `tracker_info.txt` contains IP and port details of all the trackers.
  
- **Create User Account**:  
  ```bash
  create_user <user_id> <password>
  ```

- **Login**:  
  ```bash
  login <user_id> <password>
  ```

- **Create Group**:  
  ```bash
  create_group <group_id>
  ```

- **Join Group**:  
  ```bash
  join_group <group_id>
  ```

- **Leave Group**:  
  ```bash
  leave_group <group_id>
  ```

- **List Pending Group Requests**:  
  ```bash
  list_requests <group_id>
  ```

- **Accept Group Request**:  
  ```bash
  accept_request <group_id> <user_id>
  ```

- **List All Groups**:  
  ```bash
  list_groups
  ```

- **List Sharable Files**:  
  ```bash
  list_files <group_id>
  ```

- **Upload File**:  
  ```bash
  upload_file <file_path> <group_id>
  ```

- **Download File**:  
  ```bash
  download_file <group_id> <file_name> <destination_path>
  ```

- **Show Downloads**:  
  ```bash
  show_downloads
  ```

- **Logout**:  
  ```bash
  logout
  ```

- **Stop Sharing File**:  
  ```bash
  stop_share <group_id> <file_name>
  ```

## Working Procedure
1. A tracker must always be online.
2. Users create an account to be part of the file sharing network.
3. Users can create groups and become owners of those groups.
4. Group members can share files, and other members can download them by fetching peer information from the tracker.
5. Files are divided into 512KB pieces and downloaded from multiple peers.
6. SHA1 hashing ensures the integrity of downloaded files.
7. Trackers are synchronized to maintain consistent file-sharing information.

## Assumptions
- The system assumes that at least one tracker is always online.
- Files are divided into 512KB chunks for sharing and downloading.
- Authentication is required for all client logins.

## Setup
1. Clone the repository.
2. Compile the code for both the tracker and the client using the `make` command.
3. Run the tracker and client using the commands mentioned above.

## Folder Structure
- `client/` - Contains all the client-side code.
- `tracker/` - Contains all the tracker-side code.

## Dependencies
- C/C++ with system call support
- Socket Programming
- Multi-threading

## How to Run
1. Set up at least one tracker and start it using the tracker command.
2. Run the client and perform actions like creating a user, logging in, sharing files, and downloading files using the client commands.

---

