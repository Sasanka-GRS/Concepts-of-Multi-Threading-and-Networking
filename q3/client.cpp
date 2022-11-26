#include <bits/stdc++.h>

using namespace std;

extern int shortestDistances[];
extern vector<vector<int>> bestPath;
extern int num;
extern void setup();
extern void* messagePassing(void* arg);
extern void passMessage(int dest, string msg);

void debug()
{
    cout << "here" << endl;
}

int main()
{
    setup();
    //write(0,"\n",1);

    for (int i=0; i>-1; i++)
    {
        if(i!=0)
        {
            cout << "Client >> ";
        }
        string input;

        getline(cin,input);

        //cout << "input is " << input << endl;

        vector<string> tokens;
        stringstream check1(input);
        string intermediate;

        while (getline(check1, intermediate, ' '))
        {
            tokens.push_back(intermediate);
        }

        if (tokens.size() > 3)
        {
            printf("Too many arguments given!");
            continue;
        }

        //debug();

        string pt("pt");
        string send("send");
        //debug();
        if (tokens[0] == pt)
        {
            // Given command is pt
            if (tokens.size() > 1)
            {
                cout << "Too many arguments given!" << endl;
                continue;
            }

            cout << "dest\t"
                 << "forw\t"
                 << "delay" << endl;
            for (int i = 1; i < num; i++)
            {
                cout << i << "\t" << bestPath[i][1] << "\t" << shortestDistances[i] << endl;
            }
        }
        else if(tokens[0]==send)
        {
            if(tokens.size()<3)
            {
                cout << "Too few arguments given!" << endl;
                continue;
            }
            if(tokens.size()>3)
            {
                cout << "Too many arguments given!" << endl;
                continue;
            }
            int dest = stoi(tokens[1]);
            string message = tokens[2];
            passMessage(dest,message);
        }
    }

    return 0;
}