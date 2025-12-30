#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Voter
{
    string voterid;
    string candidate;
};

class Admin
{
private:
    string username = "admin";
    string password = "123";

public:
    bool login(string u, string p)
    {
        return (u == username && p == password);
    }
};

class VotingSystem
{
private:
    vector<Voter> votes;
    int A = 0, B = 0, C = 0;

public:
    bool validID(string id)
    {
        if (id.length() != 6)
        {
            return false;
        }
        for (char c : id)
            if (!isdigit(c))
            {
                return false;
            }
        return true;
    }

    void castVote()
    {
        cin.ignore();
        Voter v;
        do
        {
            cout << "Enter 6-digit Voter ID: ";
            getline(cin, v.voterid);
            if (!validID(v.voterid))
                cout << "Invalid ID! Try again.\n";
        } while (!validID(v.voterid));

        do
        {
            cout << "Enter Candidate (A/B/C): ";
            getline(cin, v.candidate);

            if (v.candidate.length() != 1)
            {
                cout << "No such candidate\n";
                continue;
            }

            v.candidate[0] = toupper(v.candidate[0]);

            if (v.candidate != "A" && v.candidate != "B" && v.candidate != "C")
            {
                cout << "Invalid! Enter A, B, or C.\n";
            }
            else
                break;

        } while (true);

        if (v.candidate == "A")
            A++;
        else if (v.candidate == "B")
            B++;
        else if (v.candidate == "C")
            C++;

        votes.push_back(v);

        cout << "Vote Recorded Successfully!\n";
    }

    // View Vote Count
    void viewresult()
    {
        cout << "\n===== Vote Count =====\n";
        cout << "A: " << A << endl;
        cout << "B: " << B << endl;
        cout << "C: " << C << endl;
    }

    // View Voters
    void viewVoters()
    {
        if (votes.empty())
        {
            cout << "No voter records found.\n";
            return;
        }

        cout << "\n--- Voter List ---\n";
        for (auto &v : votes)
            cout << "Voter ID: " << v.voterid << " → " << v.candidate << endl;
    }

    // Delete Vote
    void deleteVote()
    {
        cin.ignore();
        Voter v;
        cout << "Enter Voter ID to delete: ";
        getline(cin, v.voterid);

        for (int i = 0; i < votes.size(); i++)
        {
            if (votes[i].voterid == v.voterid)
            {

                if (votes[i].candidate == "A")
                    A--;
                else if (votes[i].candidate == "B")
                    B--;
                else if (votes[i].candidate == "C")
                    C--;

                votes.erase(votes.begin() + i);
                cout << "✔ Vote Deleted Successfully!\n";
                return;
            }
        }
        cout << "Voter ID not found.\n";
    }
    void result()
    {
        cout << "\n=== ELECTION RESULTS ===\n";
        cout << "Candidate A: " << A << "\n";
        cout << "Candidate B: " << B << "\n";
        cout << "Candidate C: " << C << "\n";

        if (A > B && A > C)
            cout << "Winner: Candidate A!\n";
        else if (B > A && B > C)
            cout << "Winner: Candidate B!\n";
        else if (C > A && C > B)
            cout << "Winner: Candidate C!\n";
        else
            cout << "It's a tie!\n";
    }
};

int main()
{
    VotingSystem system;
    Admin admin;
    int mainChoice;

    do
    {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Voting\n2. Admin Panel\n0. Exit\nChoice: ";
        cin >> mainChoice;

        switch (mainChoice)
        {
        case 1:
        {
            int ch;
            do
            {
                cout << "\n--- VOTING MENU ---\n";
                cout << "1. Cast Vote\n2. View Results\n0. Back\nChoice: ";
                cin >> ch;

                if (ch == 1)
                {
                    system.castVote();
                }
                else if (ch == 2)
                {
                    system.viewresult();
                }
                else if (ch == 0)
                {
                    break;
                }
                else
                    cout << "Invalid choice!\n";

            } while (ch != 0);
            break;
        }

        case 2:
        {
            cin.ignore();
            string u, p;

            cout << "\n=== ADMIN LOGIN ===\n";
            cout << "Username: ";
            getline(cin, u);
            cout << "Password: ";
            getline(cin, p);

            if (admin.login(u, p))
            {
                int ch;
                do
                {
                    cout << "\n--- ADMIN PANEL ---\n";
                    cout << "1. View Voters\n2. Delete Vote\n0. Back\nChoice: ";
                    cin >> ch;

                    if (ch == 1)
                        system.viewVoters();
                    else if (ch == 2)
                        system.deleteVote();
                    else if (ch == 0)
                        break;
                    else
                        cout << "Invalid choice!\n";

                } while (ch != 0);
            }
            else
            {
                cout << "Invalid admin credentials!\n";
            }
            break;
        }

        case 0:
            cout << "Exiting...\n";
            system.result();
            break;

        default:
            cout << "Invalid option!\n";
        }

    } while (mainChoice != 0);

    return 0;
}
