#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <queue>
#include <stack>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <set>

using namespace std;

// ==================== COLOR FUNCTIONS ====================
void setColor(int c) { cout << "\033[1;" << c << "m"; }
void resetColor() { cout << "\033[0m"; }

// Convert bytes to hex
string bytesToHex(unsigned char *bytes, int length)
{
    stringstream ss;
    for (int i = 0; i < length; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

// SHA-256 with salt
string sha256(const string &input)
{
    static string SALT = "X9$!@#saltBLOCKCHAIN2025";
    string combined = input + SALT;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)combined.c_str(), combined.size(), hash);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

// ================= BLOCK CLASS ==================
class Block
{
public:
    int index;
    string voterID;
    string candidate;
    string prevHash;
    string blockHash;
    string timestamp;

    Block(int idx, string vID, string cand, string prev)
    {
        index = idx;
        voterID = vID;
        candidate = cand;
        prevHash = prev;

        auto now = chrono::system_clock::now();
        time_t t = chrono::system_clock::to_time_t(now);
        timestamp = string(ctime(&t));
        timestamp.erase(timestamp.find_last_not_of("\n") + 1);
        blockHash = sha256(toString());
    }

    string toString() const
    {
        return to_string(index) + voterID + candidate + prevHash + timestamp;
    }
};

// ================= BINARY SEARCH TREE FOR VOTERS ==================
class VoterBST
{
private:
    struct TreeNode
    {
        string id;
        bool hasVoted;
        TreeNode *left;
        TreeNode *right;
        TreeNode(string v) : id(v), hasVoted(false), left(nullptr), right(nullptr) {}
    };

    TreeNode *root;

    TreeNode *insert(TreeNode *node, const string &id)
    {
        if (!node)
            return new TreeNode(id);
        if (id < node->id)
            node->left = insert(node->left, id);
        else if (id > node->id)
            node->right = insert(node->right, id);
        // If id already exists, return existing node
        return node;
    }

    TreeNode *searchNode(TreeNode *node, const string &id)
    {
        if (!node || node->id == id)
            return node;
        if (id < node->id)
            return searchNode(node->left, id);
        return searchNode(node->right, id);
    }

    void inorder(TreeNode *node, vector<string> &result)
    {
        if (!node)
            return;
        inorder(node->left, result);
        result.push_back(node->id + (node->hasVoted ? " (Voted)" : " (Not Voted)"));
        inorder(node->right, result);
    }

    void destroyTree(TreeNode *node)
    {
        if (!node)
            return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    VoterBST() : root(nullptr) {}

    ~VoterBST()
    {
        destroyTree(root);
    }

    bool addVoter(const string &id)
    {
        if (searchNode(root, id))
            return false; // Voter already exists
        root = insert(root, id);
        return true;
    }

    bool findVoter(const string &id)
    {
        TreeNode *node = searchNode(root, id);
        return node != nullptr;
    }

    bool markVoted(const string &id)
    {
        TreeNode *node = searchNode(root, id);
        if (node && !node->hasVoted)
        {
            node->hasVoted = true;
            return true;
        }
        return false;
    }

    bool hasVoted(const string &id)
    {
        TreeNode *node = searchNode(root, id);
        return node ? node->hasVoted : false;
    }

    vector<string> getSortedVoters()
    {
        vector<string> result;
        inorder(root, result);
        return result;
    }

    void displaySortedVoters()
    {
        vector<string> voters = getSortedVoters();
        cout << "\n=== Sorted Voter List (BST) ===\n";
        if (voters.empty())
        {
            cout << "No voters registered.\n";
            return;
        }
        for (const auto &v : voters)
        {
            cout << "  " << v << endl;
        }
        cout << "Total voters: " << voters.size() << endl;
    }

    int getTotalVoters()
    {
        return getSortedVoters().size();
    }
};

// ================= GRAPH FOR POLLING STATION NETWORK ==================
class PollingGraph
{
private:
    map<string, vector<string>> adjList;
    map<string, pair<int, int>> stationCoords; // Station name -> (x, y) coordinates

public:
    void addStation(const string &station, int x = 0, int y = 0, bool silent = true)
    {
        if (adjList.find(station) == adjList.end())
        {
            adjList[station] = vector<string>();
            stationCoords[station] = make_pair(x, y);
            if (!silent)
            {
                cout << "Station '" << station << "' added at position (" << x << "," << y << ")\n";
            }
        }
    }

    void addRoad(const string &a, const string &b, bool silent = true)
    {
        if (adjList.find(a) != adjList.end() && adjList.find(b) != adjList.end())
        {
            adjList[a].push_back(b);
            adjList[b].push_back(a);
            if (!silent)
            {
                cout << "Road added between '" << a << "' and '" << b << "'\n";
            }
        }
        else
        {
            if (!silent)
            {
                cout << "Cannot add road: One or both stations don't exist.\n";
            }
        }
    }
    void displayNetwork()
    {
        cout << "\n=== Polling Station Network ===\n";
        if (adjList.empty())
        {
            cout << "No stations in the network.\n";
            return;
        }

        cout << "Stations and their connections:\n";
        for (auto &station : adjList)
        {
            cout << "\n  " << station.first << " [Pos: ("
                 << stationCoords[station.first].first << ","
                 << stationCoords[station.first].second << ")]";
            cout << " -> ";
            if (station.second.empty())
            {
                cout << "No connections";
            }
            else
            {
                for (size_t i = 0; i < station.second.size(); i++)
                {
                    cout << station.second[i];
                    if (i < station.second.size() - 1)
                        cout << ", ";
                }
            }
        }
        cout << "\n\nTotal stations: " << adjList.size() << endl;
    }

    bool stationExists(const string &station)
    {
        return adjList.find(station) != adjList.end();
    }

    vector<string> bfsShortestPath(const string &start, const string &end)
    {
        vector<string> path;
        if (!stationExists(start) || !stationExists(end))
            return path;

        map<string, string> parent;
        map<string, bool> visited;
        queue<string> q;

        q.push(start);
        visited[start] = true;
        parent[start] = "";

        while (!q.empty())
        {
            string current = q.front();
            q.pop();

            if (current == end)
            {
                // Reconstruct path
                string node = end;
                while (!node.empty())
                {
                    path.push_back(node);
                    node = parent[node];
                }
                reverse(path.begin(), path.end());
                return path;
            }

            for (const string &neighbor : adjList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }

        return path; // Empty if no path found
    }

    void displayAllStations()
    {
        cout << "\nAvailable Polling Stations:\n";
        int i = 1;
        for (auto &station : adjList)
        {
            cout << "  " << i++ << ". " << station.first << endl;
        }
    }

    int getStationCount()
    {
        return adjList.size();
    }
};

// ================= BLOCKCHAIN USING LINKED LIST ==================
class Blockchain
{
private:
    struct Node
    {
        Block data;
        Node *next;
        Node(Block b) : data(b), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    int chainSize;

public:
    Blockchain()
    {
        head = nullptr;
        tail = nullptr;
        chainSize = 0;
        addGenesisBlock();
    }

    ~Blockchain()
    {
        Node *curr = head;
        while (curr)
        {
            Node *nxt = curr->next;
            delete curr;
            curr = nxt;
        }
    }

    void addGenesisBlock()
    {
        Block genesis(0, "GENESIS", "NONE", "0");
        Node *n = new Node(genesis);
        head = tail = n;
        chainSize++;
    }

    void addBlock(string voterID, string candidate)
    {
        string prevHash = tail->data.blockHash;
        Block newBlock(chainSize, voterID, candidate, prevHash);
        Node *newNode = new Node(newBlock);
        tail->next = newNode;
        tail = newNode;
        chainSize++;

        setColor(32);
        cout << " Block #" << newBlock.index << " added to blockchain for voter " << voterID << "\n";
        resetColor();
    }

    bool validateChain()
    {
        Node *temp = head;
        while (temp && temp->next)
        {
            Node *nextNode = temp->next;
            string recalculated = sha256(nextNode->data.toString());
            if (recalculated != nextNode->data.blockHash)
                return false;
            if (nextNode->data.prevHash != temp->data.blockHash)
                return false;
            temp = temp->next;
        }
        return true;
    }

    void printChain()
    {
        Node *temp = head;
        cout << "\n======= BLOCKCHAIN LEDGER ========\n";
        cout << "Total blocks: " << chainSize << "\n";
        cout << "Chain valid: " << (validateChain() ? "YES" : "NO") << "\n\n";

        while (temp)
        {
            cout << "BLOCK #" << setw(3) << temp->data.index << "\n";

            cout << "Voter ID    : " << setw(50) << left << temp->data.voterID << " \n";
            cout << "Candidate   : " << setw(50) << left << temp->data.candidate << " \n";

            // Previous Hash
            string prevHash = temp->data.prevHash;
            if (prevHash.length() > 64)
                prevHash = prevHash.substr(0, 64);
            cout << "PrevHash:";
            cout << " " << prevHash << " \n";

            // Current Block Hash
            string blockHash = temp->data.blockHash;
            if (blockHash.length() > 64)
                blockHash = blockHash.substr(0, 64);
            cout << "Hash:";
            cout << " " << blockHash << " \n";

            cout << " Timestamp: " << setw(52) << left << temp->data.timestamp << " \n";
            cout << "" << endl;
            if (temp->next)
                cout << " \n";

            temp = temp->next;
        }
        cout << "\n═══════════════════════════════════════════════════════════════════════════════════\n";
    }

    void saveToFile()
    {
        ofstream fout("blockchain_ledger.txt");
        if (!fout)
            throw runtime_error("Unable to open file to save blockchain.");

        Node *temp = head;
        fout << "Index|VoterID|Candidate|PrevHash|BlockHash|Timestamp\n";
        fout << "----------------------------------------------------\n";

        while (temp)
        {
            fout << temp->data.index << "|"
                 << temp->data.voterID << "|"
                 << temp->data.candidate << "|"
                 << temp->data.prevHash << "|"
                 << temp->data.blockHash << "|"
                 << temp->data.timestamp << "\n";
            temp = temp->next;
        }
        fout.close();

        setColor(32);
        cout << "Blockchain saved to 'blockchain_ledger.txt'\n";
        resetColor();
    }

    int getVoteCountForCandidate(const string &candidate)
    {
        int count = 0;
        Node *temp = head->next; // Skip genesis block
        while (temp)
        {
            if (temp->data.candidate == candidate)
                count++;
            temp = temp->next;
        }
        return count;
    }

    int getTotalVotes()
    {
        return chainSize - 1; // Exclude genesis block
    }
};

// ================= VOTING SYSTEM ==================
class VotingSystem
{
private:
    Blockchain chain;
    queue<string> voterQueue;
    stack<string> adminActions;
    unordered_set<string> votedIDs;
    vector<pair<string, string>> pendingVotes;
    map<string, int> voteCounts;

    // New DS: Integrated BST and Graph
    VoterBST voterTree;
    PollingGraph stationGraph;

    // Current voting station and station data
    string currentStation;
    map<string, map<string, int>> stationVotes; // station -> candidate -> count
    map<string, string> voterToStation;         // voterID -> station

    string adminPassword = "@78/;ljcxz";

    bool validID(const string &voterID)
    {
        if (voterID.length() != 6)
            return false;
        return all_of(voterID.begin(), voterID.end(), ::isdigit);
    }

    void setupDefaultNetwork()
    {
        // Create a network of polling stations (silent mode)
        stationGraph.addStation("StationA", 0, 0, true); // true = silent
        stationGraph.addStation("StationB", 10, 0, true);
        stationGraph.addStation("StationC", 5, 10, true);
        stationGraph.addStation("StationD", 15, 5, true);
        stationGraph.addStation("StationE", -5, 5, true);

        // Connect stations (silent mode)
        stationGraph.addRoad("StationA", "StationB", true);
        stationGraph.addRoad("StationA", "StationE", true);
        stationGraph.addRoad("StationB", "StationC", true);
        stationGraph.addRoad("StationB", "StationD", true);
        stationGraph.addRoad("StationC", "StationD", true);

        // Initialize station vote tracking
        for (auto &station : vector<string>{"StationA", "StationB", "StationC", "StationD", "StationE"})
        {
            stationVotes[station]["A"] = 0;
            stationVotes[station]["B"] = 0;
            stationVotes[station]["C"] = 0;
        }
    }

public:
    VotingSystem()
    {
        // Initialize vote counts
        voteCounts["A"] = 0;
        voteCounts["B"] = 0;
        voteCounts["C"] = 0;

        // Setup default polling network
        setupDefaultNetwork();

        currentStation = "StationA";

        setColor(36);
        cout << "Voting System Initialized\n";
        cout << "Default polling network created\n";
        cout << "Current station: " << currentStation << "\n";
        resetColor();
    }

    // ================= BST OPERATIONS ==================
    bool registerVoter(const string &voterID)
    {
        if (!validID(voterID))
        {
            setColor(31);
            cout << "Invalid voter ID. Must be 6 digits.\n";
            resetColor();
            return false;
        }

        if (!voterTree.addVoter(voterID))
        {
            setColor(33);
            cout << "Voter " << voterID << " is already registered.\n";
            resetColor();
            return false;
        }

        setColor(32);
        cout << "Voter " << voterID << " registered successfully.\n";
        resetColor();
        return true;
    }

    void showSortedVoters()
    {
        voterTree.displaySortedVoters();
    }

    bool checkVoterStatus(const string &voterID)
    {
        if (!voterTree.findVoter(voterID))
        {
            cout << "Voter " << voterID << " is not registered.\n";
            return false;
        }

        if (voterTree.hasVoted(voterID))
        {
            cout << "Voter " << voterID << " has already voted.\n";
            return true;
        }
        else
        {
            cout << "Voter " << voterID << " is registered but has not voted.\n";
            return false;
        }
    }

    // ================= GRAPH OPERATIONS ==================
    void displayStationNetwork()
    {
        stationGraph.displayNetwork();
    }

    void findRouteBetweenStations()
    {
        string start, end;
        cout << "Enter starting station: ";
        cin >> start;
        cout << "Enter destination station: ";
        cin >> end;

        vector<string> path = stationGraph.bfsShortestPath(start, end);

        if (path.empty())
        {
            setColor(31);
            cout << "No path found between " << start << " and " << end << "\n";
            resetColor();
        }
        else
        {
            setColor(32);
            cout << "Shortest path from " << start << " to " << end << ":\n";
            resetColor();
            cout << "  ";
            for (size_t i = 0; i < path.size(); i++)
            {
                cout << path[i];
                if (i < path.size() - 1)
                    cout << " → ";
            }
            cout << "\n  Total stations: " << path.size() << endl;
        }
    }

    void addNewStation()
    {
        string name;
        int x, y;
        cout << "Enter station name: ";
        cin >> name;
        cout << "Enter X coordinate: ";
        cin >> x;
        cout << "Enter Y coordinate: ";
        cin >> y;

        // false = show messages for user-added stations
        stationGraph.addStation(name, x, y, false);

        // Initialize vote counts for new station
        stationVotes[name]["A"] = 0;
        stationVotes[name]["B"] = 0;
        stationVotes[name]["C"] = 0;
    }

    void connectStations()
    {
        string a, b;
        stationGraph.displayAllStations();
        cout << "\nEnter first station: ";
        cin >> a;
        cout << "Enter second station: ";
        cin >> b;

        // false = show messages for user-added connections
        stationGraph.addRoad(a, b, false);
    }
    // ================= VOTING OPERATIONS ==================
    void enqueueVoter(const string &voterID)
    {
        if (!validID(voterID))
        {
            setColor(31);
            cout << "Invalid voter ID format.\n";
            resetColor();
            return;
        }

        if (!voterTree.findVoter(voterID))
        {
            setColor(33);
            cout << "Voter not registered. Registering now...\n";
            resetColor();
            registerVoter(voterID);
        }

        voterQueue.push(voterID);
        adminActions.push("Enqueue:" + voterID);

        setColor(32);
        cout << "Voter " << voterID << " added to queue.\n";
        resetColor();
        cout << "Queue size: " << voterQueue.size() << endl;
    }

    void castVoteFromQueue()
    {
        try
        {
            if (voterQueue.empty())
                throw runtime_error("Queue is empty!");

            string voterID = voterQueue.front();
            cout << "\nProcessing voter: " << voterID << " at " << currentStation << endl;

            // Check if already voted using BST
            if (voterTree.hasVoted(voterID))
            {
                setColor(31);
                cout << "✗ Voter " << voterID << " has already voted!\n";
                resetColor();
                voterQueue.pop(); // Remove from queue anyway
                adminActions.push("DuplicateAttempt:" + voterID);
                return;
            }

            string cand;
            do
            {
                cout << "Candidates: A, B, C\n";
                cout << "Enter candidate for voter " << voterID << ": ";
                cin >> cand;
                cand[0] = toupper(cand[0]);
            } while (cand != "A" && cand != "B" && cand != "C");

            // Mark as voted in BST
            voterTree.markVoted(voterID);

            // Record station information
            voterToStation[voterID] = currentStation;
            stationVotes[currentStation][cand]++;

            pendingVotes.push_back({voterID, cand});
            votedIDs.insert(voterID);
            adminActions.push("CastPending:" + voterID + ":" + cand + "@" + currentStation);

            voterQueue.pop();

            setColor(32);
            cout << "Vote recorded for " << voterID << " at " << currentStation
                 << " (pending blockchain submission)\n";
            resetColor();
        }
        catch (exception &e)
        {
            setColor(31);
            cout << "Error: " << e.what() << endl;
            resetColor();
        }
    }

    void submitPendingToChain()
    {
        if (pendingVotes.empty())
        {
            cout << "No pending votes to submit.\n";
            return;
        }

        setColor(36);
        cout << "\nSubmitting " << pendingVotes.size() << " votes to blockchain...\n";
        resetColor();

        for (auto &p : pendingVotes)
        {
            chain.addBlock(p.first, p.second);
            voteCounts[p.second]++;
        }

        pendingVotes.clear();

        setColor(32);
        cout << "All pending votes submitted to blockchain.\n";
        resetColor();
    }

    // ================= STATION OPERATIONS ==================
    void switchPollingStation()
    {
        cout << "\nCurrent station: " << currentStation << endl;
        cout << "Available stations:\n";
        stationGraph.displayAllStations();

        string newStation;
        cout << "Enter station to switch to: ";
        cin >> newStation;

        if (stationGraph.stationExists(newStation))
        {
            currentStation = newStation;
            setColor(32);
            cout << "Switched to polling station: " << currentStation << endl;
            resetColor();
        }
        else
        {
            setColor(31);
            cout << "Station '" << newStation << "' does not exist!\n";
            resetColor();
        }
    }

    void displayStationResults()
    {
        cout << "\n=== POLLING STATION RESULTS ===\n";
        for (auto &station : stationVotes)
        {
            if (stationGraph.stationExists(station.first))
            {
                cout << "\n"
                     << station.first << ":\n";
                cout << "  Candidate A: " << station.second["A"] << " votes\n";
                cout << "  Candidate B: " << station.second["B"] << " votes\n";
                cout << "  Candidate C: " << station.second["C"] << " votes\n";
                cout << "  Total: " << (station.second["A"] + station.second["B"] + station.second["C"])
                     << " votes\n";
            }
        }
    }

    // ================= ADMIN OPERATIONS ==================
    bool adminLogin()
    {
        string pass;
        setColor(33);
        cout << "Enter Admin Password: ";
        resetColor();
        cin >> pass;

        if (pass == adminPassword)
        {
            setColor(32);
            cout << "Admin access granted.\n";
            resetColor();
            return true;
        }
        else
        {
            setColor(31);
            cout << " Access denied!\n";
            resetColor();
            return false;
        }
    }

    void displayStatistics()
    {
        cout << "\n=== VOTING STATISTICS ===\n";
        cout << "Current Station: " << currentStation << endl;
        cout << "Total registered voters (BST): " << voterTree.getTotalVoters() << endl;
        cout << "Votes in queue: " << voterQueue.size() << endl;
        cout << "Pending votes to submit: " << pendingVotes.size() << endl;
        cout << "Total votes cast (blockchain): " << chain.getTotalVotes() << endl;

        cout << "\nOverall Candidate Results:\n";
        cout << "  Candidate A: " << chain.getVoteCountForCandidate("A") << " votes\n";
        cout << "  Candidate B: " << chain.getVoteCountForCandidate("B") << " votes\n";
        cout << "  Candidate C: " << chain.getVoteCountForCandidate("C") << " votes\n";

        cout << "\nStation Distribution:\n";
        for (auto &station : stationVotes)
        {
            if (stationGraph.stationExists(station.first))
            {
                int total = station.second["A"] + station.second["B"] + station.second["C"];
                cout << "  " << station.first << ": " << total << " votes\n";
            }
        }

        cout << "\nPolling Stations: " << stationGraph.getStationCount() << " stations\n";
    }

    void adminPanel()
    {
        if (!adminLogin())
            return;

        int ch;
        while (true)
        {
            cout << "\n--- ADMIN PANEL ---\n";
            cout << "1. View Blockchain\n";
            cout << "2. Validate Blockchain\n";
            cout << "3. Save Blockchain\n";
            cout << "4. View Statistics\n";
            cout << "5. View Sorted Voters\n";
            cout << "6. View Station Network\n";
            cout << "7. Check Voter Status\n";
            cout << "8. Back to Main Menu\n";
            cout << "Choice: ";

            if (!(cin >> ch))
            {
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (ch == 8)
                break;

            switch (ch)
            {
            case 1:
                chain.printChain();
                break;
            case 2:
                cout << (chain.validateChain() ? " Blockchain is VALID\n" : " Blockchain is INVALID\n");
                break;
            case 3:
                chain.saveToFile();
                break;
            case 4:
                displayStatistics();
                break;
            case 5:
                showSortedVoters();
                break;
            case 6:
                displayStationNetwork();
                break;
            case 7:
            {
                string id;
                cout << "Enter Voter ID to check: ";
                cin >> id;
                checkVoterStatus(id);
                break;
            }
            default:
                cout << "Invalid choice.\n";
            }
        }
    }

    // ================= GRAPH MENU ==================
    void graphMenu()
    {
        int ch;
        while (true)
        {
            cout << "\n--- POLLING NETWORK MENU ---\n";
            cout << "1. View Station Network\n";
            cout << "2. Find Shortest Route\n";
            cout << "3. Add New Station\n";
            cout << "4. Connect Two Stations\n";
            cout << "5. List All Stations\n";
            cout << "6. Back to Main Menu\n";
            cout << "Choice: ";

            if (!(cin >> ch))
            {
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (ch == 6)
                break;

            switch (ch)
            {
            case 1:
                displayStationNetwork();
                break;
            case 2:
                findRouteBetweenStations();
                break;
            case 3:
                addNewStation();
                break;
            case 4:
                connectStations();
                break;
            case 5:
                stationGraph.displayAllStations();
                break;
            default:
                cout << "Invalid choice.\n";
            }
        }
    }

    // ================= STATION MENU ==================
    void stationMenu()
    {
        int ch;
        while (true)
        {
            cout << "\n--- POLLING STATION MENU ---\n";
            cout << "Current Station: " << currentStation << "\n";
            cout << "1. Switch Polling Station\n";
            cout << "2. View Station Results\n";
            cout << "3. View Station Network\n";
            cout << "4. Find Route to Another Station\n";
            cout << "5. Back to Main Menu\n";
            cout << "Choice: ";

            cin >> ch;

            if (ch == 5)
                break;

            switch (ch)
            {
            case 1:
                switchPollingStation();
                break;
            case 2:
                displayStationResults();
                break;
            case 3:
                displayStationNetwork();
                break;
            case 4:
            {
                string destination;
                cout << "Current station: " << currentStation << endl;
                cout << "Enter destination station: ";
                cin >> destination;

                vector<string> path = stationGraph.bfsShortestPath(currentStation, destination);

                if (path.empty())
                {
                    setColor(31);
                    cout << "No route from " << currentStation << " to " << destination << "\n";
                    resetColor();
                }
                else
                {
                    setColor(32);
                    cout << "Route from " << currentStation << " to " << destination << ":\n";
                    resetColor();
                    cout << "  ";
                    for (size_t i = 0; i < path.size(); i++)
                    {
                        if (path[i] == currentStation)
                            setColor(33);
                        cout << path[i];
                        resetColor();

                        if (i < path.size() - 1)
                            cout << " → ";
                    }
                    cout << "\n  Distance: " << (path.size() - 1) << " stations\n";
                }
                break;
            }
            default:
                cout << "Invalid choice.\n";
            }
        }
    }

    // ================= ELECTION RESULTS ==================
    void declareElectionResults()
    {
        cout << "\n";
        setColor(36);
        cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║                         ELECTION RESULTS DECLARATION                         ║\n";
        cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
        resetColor();

        int totalVotes = chain.getTotalVotes();
        if (totalVotes == 0)
        {
            cout << "\nNo votes have been cast yet!\n";
            return;
        }

        // Get vote counts
        int votesA = chain.getVoteCountForCandidate("A");
        int votesB = chain.getVoteCountForCandidate("B");
        int votesC = chain.getVoteCountForCandidate("C");

        // Display vote counts
        cout << endl;
        cout << "┌─────────────────────────────────────────────────────────────┐\n";
        cout << "│                    VOTE DISTRIBUTION                          │\n";
        cout << "├───────────────────────────────────────────────────────────────┤\n";
        cout << "│ Candidate A: " << setw(6) << votesA << " votes                │\n";
        cout << "│ Candidate B: " << setw(6) << votesB << " votes                │\n";
        cout << "│ Candidate C: " << setw(6) << votesC << " votes                │\n";
        cout << "├───────────────────────────────────────────────────────────────┤\n";
        cout << "│ Total Votes: " << setw(6) << totalVotes << " votes            │\n";
        cout << "└───────────────────────────────────────────────────────────────┘\n";

        // Determine winner
        cout << "\nELECTION RESULTS:\n";

        vector<pair<string, int>> candidates = {
            {"A", votesA},
            {"B", votesB},
            {"C", votesC}};

        // Sort by votes (highest first)
        sort(candidates.begin(), candidates.end(),
             [](const pair<string, int> &a, const pair<string, int> &b)
             {
                 return a.second > b.second;
             });

        // Check for ties
        if (candidates[0].second == candidates[1].second &&
            candidates[0].second == candidates[2].second)
        {
            // Three-way tie
            setColor(33); // Yellow for tie
            cout << "IT'S A THREE-WAY TIE! All candidates have "
                 << candidates[0].second << " votes.\n";
            resetColor();
        }
        else if (candidates[0].second == candidates[1].second)
        {
            // Two-way tie
            setColor(33); // Yellow for tie
            cout << "IT'S A TIE! Candidates " << candidates[0].first
                 << " and " << candidates[1].first
                 << " both have " << candidates[0].second << " votes.\n";
            resetColor();

            // Show margin over third place
            if (candidates.size() > 2)
            {
                int margin = candidates[0].second - candidates[2].second;
                cout << "   Lead over " << candidates[2].first << ": " << margin << " votes\n";
            }
        }
        else
        {
            // Clear winner
            setColor(32); // Green for winner
            cout << " WINNER: Candidate " << candidates[0].first
                 << " with " << candidates[0].second << " votes!\n";
            resetColor();

            // Calculate and show margin of victory
            int margin = candidates[0].second - candidates[1].second;
            cout << "   Margin of victory: " << margin << " votes\n";

            // Show percentage margin if possible
            if (totalVotes > 0)
            {
                float percentageMargin = (margin * 100.0) / totalVotes;
                cout << "   (" << fixed << setprecision(1) << percentageMargin << "% lead)\n";
            }
        }

        cout << "\nDETAILED COMPARISON:\n";
        for (size_t i = 0; i < candidates.size(); i++)
        {
            cout << "   " << (i + 1) << ". Candidate " << candidates[i].first
                 << ": " << candidates[i].second << " votes";

            if (i > 0 && candidates[0].second > 0)
            {
                float percentage = (candidates[i].second * 100.0) / candidates[0].second;
                cout << " (" << fixed << setprecision(1) << percentage << "% of winner)";
            }
            cout << endl;
        }
    }

    void showQuickResults()
    {
        int totalVotes = chain.getTotalVotes();
        if (totalVotes == 0)
        {
            cout << "No votes cast yet.\n";
            return;
        }

        int votesA = chain.getVoteCountForCandidate("A");
        int votesB = chain.getVoteCountForCandidate("B");
        int votesC = chain.getVoteCountForCandidate("C");

        cout << "\nQUICK RESULTS:\n";
        cout << "Candidate A: " << votesA << " votes\n";
        cout << "Candidate B: " << votesB << " votes\n";
        cout << "Candidate C: " << votesC << " votes\n";

        // Find leader
        string leader = "";
        int maxVotes = max({votesA, votesB, votesC});

        if (maxVotes == votesA)
            leader += "A ";
        if (maxVotes == votesB)
            leader += "B ";
        if (maxVotes == votesC)
            leader += "C ";

        if (leader.find(' ') != string::npos && leader.find_last_of(' ') != 0)
        {
            cout << "Current leaders: " << leader << "(Tie)\n";
        }
        else
        {
            cout << "Current leader: " << leader << "\n";
        }
    }

    // Check if election can be concluded (admin-only)
    void concludeElection()
    {
        if (!adminLogin())
        {
            cout << "Access denied! Admin rights required to conclude election.\n";
            return;
        }

        cout << "\n";
        setColor(33);
        cout << " WARNING: This will officially conclude the election!\n";
        cout << "   No more votes will be accepted after this.\n";
        resetColor();

        char confirm;
        cout << "Are you sure you want to conclude the election? (Y/N): ";
        cin >> confirm;

        if (toupper(confirm) == 'Y')
        {
            declareElectionResults();

            cout << "\n Election officially concluded!\n";

            // Save final results to file
            ofstream finalResults("final_election_results.txt");
            if (finalResults)
            {
                finalResults << "FINAL ELECTION RESULTS\n";
                finalResults << "=======================\n";
                // Replace line 1235 with:
                time_t currentTime = time(nullptr);
                finalResults << "Timestamp: " << ctime(&currentTime);

                finalResults << "Total Votes: " << chain.getTotalVotes() << "\n";
                finalResults << "Candidate A: " << chain.getVoteCountForCandidate("A") << " votes\n";
                finalResults << "Candidate B: " << chain.getVoteCountForCandidate("B") << " votes\n";
                finalResults << "Candidate C: " << chain.getVoteCountForCandidate("C") << " votes\n";
                finalResults.close();

                setColor(32);
                cout << "Final results saved to 'final_election_results.txt'\n";
                resetColor();
            }
        }
        else
        {
            cout << "Election conclusion cancelled.\n";
        }
    }
    // ================= GETTERS ==================
    string getCurrentStation() const { return currentStation; }
    int getQueueSize() const { return voterQueue.size(); }
    int getPendingVotes() const { return pendingVotes.size(); }
};

// ==================== ASCII TITLE ====================
void printTitle()
{
    setColor(36); // Cyan color
    cout << R"( 
  ____  _            _        _            _                   _   _                  
 | __ )| | ___   ___| | _____| |__   __ _|(_)_ __ __   __ ___ | |_(_)_ __   __ _ 
 |  _ \| |/ _ \ / __| |/ | __| '_ \ / _` || | '_ \\ \ / // _ \| __| | '_ \ / _` |
 | |_) | | (_) | (__|  <| (__| | | | (_| || | | | |\ v /| (_) | |_| | | | | (_| |   
 |____/|_|\___/ \___|_|\_|___/_| |_|\__,_||_|_| |_| \_/  \___/ \__|_|_| |_|\__, |
                                                                            |___/
                                            
 
)" << endl;
    resetColor();
}

// ================= MAIN ==================
int main()
{
    printTitle();

    VotingSystem system;
    int choice;

    while (true)
    {
        cout << "\n═══════════════════════════════════════════════════\n";
        cout << "                    MAIN MENU                      \n";
        cout << "Current Station: " << system.getCurrentStation() << "\n";
        cout << "═══════════════════════════════════════════════════\n";
        cout << "1. Register New Voter \n";
        cout << "2. Add Voter to Queue\n";
        cout << "3. Cast Vote from Queue\n";
        cout << "4. Submit Pending Votes to Blockchain\n";
        cout << "5. Polling Station Operations\n";
        cout << "6. Admin Panel\n";
        cout << "7. Switch Polling Station\n";
        cout << "8. View Station Results\n";
        cout << "9. View Sorted Voter List \n";
        cout << "10. View Election Results\n";
        cout << "11. Exit\n";
        cout << "═══════════════════════════════════════════════════\n";
        cout << "Choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            string id;
            cout << "Enter 6-digit Voter ID: ";
            cin >> id;
            system.registerVoter(id);
            break;
        }
        case 2:
        {
            string id;
            cout << "Enter Voter ID to add to queue: ";
            cin >> id;
            system.enqueueVoter(id);
            break;
        }
        case 3:
            system.castVoteFromQueue();
            break;
        case 4:
            system.submitPendingToChain();
            break;
        case 5:
            system.stationMenu();
            break;
        case 6:
            system.adminPanel();
            break;
        case 7:
            system.switchPollingStation();
            break;
        case 8:
            system.displayStationResults();
            break;
        case 9:
            system.showSortedVoters();
            break;
        case 10:  
            concludeElection();
            break;
        case 11:
            setColor(32);
            cout << "\nThank you for using the Blockchain Voting System!\n";
            resetColor();
            return 0;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }
}
