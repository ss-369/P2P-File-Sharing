# AOS Assignment 4
## Distributed File Sharing System

This system is designed for group-based file sharing, enabling users to share and download files within their affiliated groups. The download process is optimized for parallelism, allowing the retrieval of multiple file pieces simultaneously from various peers. Additionally, the system offers robust support for multithreaded client, server, and tracker components.

## Tracker:
-It stores client information, including the files they share, to facilitate peer-to-peer communication.
-The "tracker_info.txt" file holds IP and port details of all the trackers in the network.

## Client:
-It acquires files from multiple peers and promptly becomes a leecher by sharing the downloaded content.
### Prerequisites
1. G++ compiler
   * ```sudo apt-get install g++```

### run makefile
     ``` make clean```
        ``` make ```

#### To run the Tracker 

```
./tracker_program tracker/tracker_info.txt tracker_no 

eg : ./tracker_program tracker/tracker_info.txt 1

```

#### To run the Client

```

./client_program <IP>:<PORT> client/tracker_info.txt

eg : ./client_program 127.0.0.1:7604 client/tracker_info.txt

```




## System Operation Requirements

1. **Tracker Availability:** The network ensures that at least one tracker remains online at all times.

2. **User Registration:** Users must create an account by providing a unique user ID and password to access the network.

3. **Group Management:** Clients have the ability to create and own multiple groups within the network.

4. **Group Access:** To download files, users must be members of the respective group.

5. **Joining a Group:** Users initiate join requests to become part of a group.

6. **Accept/Reject Requests:** Group owners have the authority to accept or reject incoming join requests.

7. **File Access:** Once part of a group, clients can view a list of files that can be shared within that group.

8. **File Sharing:** Users can contribute files to a group. However, only the client's IP and port information is stored on the tracker.

9. **File Download:** Clients can request file downloads by issuing commands to the tracker, specifying the group name and file name. The tracker responds with information about group members sharing that specific file.

10. **Peer Communication:** After obtaining peer information from the tracker, clients establish communication with peers to identify which parts of the file each peer possesses. Clients can then make informed decisions about which parts to download from which peers, utilizing a custom Piece Selection Algorithm.

11. **Immediate Sharing:** Downloaded file pieces are made available for sharing instantly.

12. **Logout Behavior:** When clients log out, they temporarily suspend the sharing of files until the next login.

13. **Tracker Synchronization:** All trackers in the network must remain in sync with each other to ensure consistent operation.

## Functionality for Clients

The following commands are available for clients to interact with the network:

- `create_user <user_id> <passwd>`: Create a user account.
- `login <user_id> <passwd>`: Log in to the network.
- `create_group <group_id>`: Create a new group and become the owner.
- `join_group <group_id>`: Join an existing group.
- `leave_group <group_id>`: Leave a group.
- `list_requests <group_id>`: List pending join requests for a group.
- `accept_request <group_id> <user_id>`: Accept a user's request to join a group.
- `list_groups`: List all groups in the network.
- `list_files <group_id>`: List all shareable files within a group.
- `upload_file <file_path> <group_id>`: Upload a file to a group.
- `download_file <group_id> <file_name> <destination_path>`: Request to download a file from a group.
- `logout`: Log out of the network.
* Show_downloads: `show_downloads`
  * Output format:
    * [D] [grp_id] filename
    * [C] [grp_id] filename
    * D(Downloading), C(Complete)
- `stop_share <group_id> <file_name>`: Stop sharing a file in a group.
