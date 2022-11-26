#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;

int adjacency[1000][1000];
int shortestDistances[1000];
vector<int> path;
vector<vector<int>> bestPath(1000);
int parents[1000];
int num;
int progress = 0;
int here=0;

typedef struct thread_details
{
    int final,fwd,id;
    string message;
} td;

int minDistance(int dist[], bool sptSet[], int n)
{
    int min = INT_MAX, min_index;
 
    for (int v=0; v<n; v++)
    {
        if (sptSet[v] == false && dist[v] <= min)
        {
            min = dist[v];
            min_index = v;
        }
    }
 
    return min_index;
}

void dijkstra(int startVertex, int n)
{
    vector<bool> added(n);

    for (int vertexIndex = 0; vertexIndex < n; vertexIndex++) 
    {
        shortestDistances[vertexIndex] = INT_MAX;
        added[vertexIndex] = false;
    }

    shortestDistances[startVertex] = 0;
    parents[startVertex] = -1;

    for (int i = 1; i < n; i++) 
    {
        int nearestVertex = -1;
        int shortestDistance = INT_MAX;
        for (int vertexIndex = 0; vertexIndex < n; vertexIndex++) 
        {
            if (!added[vertexIndex] && shortestDistances[vertexIndex] < shortestDistance) 
            {
                nearestVertex = vertexIndex;
                shortestDistance = shortestDistances[vertexIndex];
            }
        }

        added[nearestVertex] = true;
 
        for (int vertexIndex = 0; vertexIndex < n; vertexIndex++) 
        {
            int edgeDistance = adjacency[nearestVertex][vertexIndex];
            if (edgeDistance > 0 && ((shortestDistance + edgeDistance) < shortestDistances[vertexIndex])) 
            {
                parents[vertexIndex] = nearestVertex;
                shortestDistances[vertexIndex] = shortestDistance + edgeDistance;
            }
        }
    }
}

void printPath(int currentVertex)
{
    if (currentVertex == -1) {
        return;
    }
    printPath(parents[currentVertex]);
    bestPath[here].push_back(currentVertex);
}

void find_table(int n)
{
    dijkstra(0,n);
    for(int i=1; i<n; i++)
    {
        here = i;
        printPath(i);
    }
}

void setup()
{
    int n,m;
    cin >> n >> m;
    num = n;

    for(int i=0; i<m; i++)
    {
        int a,b,w;
        cin >> a >> b >> w;
        adjacency[a][b] = w;
        adjacency[b][a] = w;
    }

    find_table(n);
}

mutex locks[1000];

void messagePassing(td* arg)
{
    int serverNum = ((struct thread_details *)arg)->id;
    int destServer = ((struct thread_details *)arg)->final;
    int nextServer = ((struct thread_details *)arg)->fwd;
    string messageHere = ((struct thread_details *)arg)->message;

    if(serverNum==0)
    {
        progress++;
        sleep(1);
        cout << "Data received at node : " << serverNum << "; Source : 0; Destination : " << destServer << "; Forwarded_Destination : " << nextServer << "; Message : \"" << messageHere << "\";" << endl;
        sleep(1);
        locks[progress].unlock();
        return;
    }
    else if(serverNum==destServer)
    {
        locks[bestPath[destServer].size()-1].lock();
        cout << "Data received at node : " << serverNum << "; Source : 0; Destination : " << destServer << "; Forwarded_Destination : " << "None" << "; Message : \"" << messageHere << "\";" << endl;
        sleep(1);
        return;
    }
    else
    {
        locks[progress].lock();
        
        cout << "Data received at node: " << serverNum << "; Source : 0; Destination : " << destServer << "; Forwarded_Destination : " << nextServer << "; Message : \"" << messageHere << "\";" << endl;
        progress++;
        sleep(1);
        locks[progress].unlock();
    }

    return;
}


void passMessage(int dest, string msg)
{
    if(dest>num-1)
    {
        cout << "No such node exists!" << endl;
        return;
    }

    for(int i=0; i<1000; i++)
    {
        locks[i].unlock();
    }

    progress = 0;

    int servNum = bestPath[dest].size();
    
    thread th[servNum];
    for(int i=0; i<servNum; i++)
    {
        locks[i].lock();
    }

    locks[0].unlock();

    cout << "Data sent to server source node : 0 : \"" << msg << "\"" << endl;
    
    for(int i=0; i<servNum; i++)
    {
        td *thread_input = (td *)(malloc(sizeof(td)));
        thread_input->id = bestPath[dest][i];
        thread_input->final = dest;
        if(i==servNum-1)
        {
            thread_input->fwd = -1;
        }
        else
        {
            thread_input->fwd = bestPath[dest][i+1];
        }
        thread_input->message = msg;
        th[i] = thread(messagePassing,thread_input);
    }

    for(int i=0; i<servNum; i++)
    {
        th[i].join();
    }

    cout << "Data received at server destination node : " << dest << " : \"" << msg << "\"" << endl;
}
