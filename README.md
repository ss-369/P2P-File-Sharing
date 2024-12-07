# Assignment 4: Distributed File Sharing System

This system facilitates group-based file sharing, allowing users to share and download files within groups. The download mechanism is designed for parallelism, enabling the simultaneous retrieval of file pieces from multiple peers. The system also supports multithreaded components for the client, server, and tracker.

---

## System Components

### **Tracker**
- Maintains information about clients and the files they share to enable peer-to-peer communication.
- Stores tracker details (IP and port) in a file named `tracker_info.txt`.

### **Client**
- Downloads files from peers and becomes a leecher by sharing the downloaded content immediately.

---

## Prerequisites
1. **G++ Compiler**
   - Install using:
     ```bash
     sudo apt-get install g++
     ```

---

## Build Instructions
1. Clean the previous build:
   ```bash
   make clean
   ```
2. Build the system:
   ```bash
   make
   ```

---

## Execution Instructions

### **To Run the Tracker**
```bash
./tracker_program <tracker_info_path> <tracker_no>

# Example
./tracker_program tracker/tracker_info.txt 1
```

### **To Run the Client**
```bash
./client_program <IP>:<PORT> <tracker_info_path>

# Example
./client_program 127.0.0.1:7604 client/tracker_info.txt
```

---

## System Requirements & Features

1. **Tracker Uptime:** At least one tracker must remain online at all times.
2. **User Accounts:** Users register with a unique user ID and password to access the network.
3. **Group Management:**
   - Users can create and own multiple groups.
   - Group owners can manage member requests.
4. **Group Membership:** Only group members can download files shared within the group.
5. **Join Requests:** Users request to join groups; group owners can accept or reject these requests.
6. **File Listings:** Members can view shareable files within their groups.
7. **File Sharing:**
   - Users can upload files to groups.
   - Only IP and port details are stored on the tracker.
8. **File Downloading:** Clients request downloads by specifying group and file names. The tracker provides peer information for file sharing.
9. **Piece Selection Algorithm:** Clients determine which file pieces to download from which peers, optimizing parallel downloads.
10. **Immediate Sharing:** Downloaded file pieces are instantly available for sharing.
11. **Logout Behavior:** Logged-out clients temporarily suspend file sharing until they log back in.
12. **Tracker Synchronization:** All trackers in the network must stay synchronized for consistency.

---

## Client Commands

| **Command**                        | **Description**                                                    |
|------------------------------------|--------------------------------------------------------------------|
| `create_user <user_id> <passwd>`   | Create a new user account.                                         |
| `login <user_id> <passwd>`         | Log in to the network.                                             |
| `create_group <group_id>`          | Create a new group and become the owner.                          |
| `join_group <group_id>`            | Request to join an existing group.                                |
| `leave_group <group_id>`           | Leave a group.                                                    |
| `list_requests <group_id>`         | View pending join requests for a group.                           |
| `accept_request <group_id> <user_id>` | Approve a user's request to join a group.                      |
| `list_groups`                      | List all available groups in the network.                         |
| `list_files <group_id>`            | List all shareable files within a group.                          |
| `upload_file <file_path> <group_id>` | Upload a file to a group.                                       |
| `download_file <group_id> <file_name> <destination_path>` | Download a file from a group.          |
| `logout`                           | Log out of the network.                                            |
| `show_downloads`                   | View download status:                                              |
|                                    | **Output Format:** `[D] [grp_id] filename` or `[C] [grp_id] filename` |
|                                    | `D`: Downloading, `C`: Complete                                    |
| `stop_share <group_id> <file_name>`| Stop sharing a specific file within a group.                      |

---

## System Example
1. **Tracker Initialization**
   ```bash
   ./tracker_program tracker/tracker_info.txt 1
   ```

2. **Client Login**
   ```bash
   ./client_program 127.0.0.1:7604 client/tracker_info.txt
   ```

3. **Commands for File Sharing**
   - Create a user:
     ```bash
     create_user user123 pass123
     ```
   - Log in:
     ```bash
     login user123 pass123
     ```
   - Create a group:
     ```bash
     create_group group1
     ```
   - Upload a file:
     ```bash
     upload_file /path/to/file.txt group1
     ```
   - Download a file:
     ```bash
     download_file group1 file.txt /destination/path
     ```
   - Show downloads:
     ```bash
     show_downloads
     ```

--- 

This system ensures a seamless experience for distributed file sharing with robust parallel downloading and synchronization mechanisms.
