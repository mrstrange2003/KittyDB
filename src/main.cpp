#include<iostream>
#include<string>
#include<algorithm>
using namespace std;
int main(){
    cout<<"Welcome to KittyDB"<<endl;
    cout<<"Type 'exit' to quit"<<endl;
    
    while(true){
        cout<<"KittyDB > ";
        string command;
        getline(cin, command);

        if(command.empty()){
            continue;
        }

        string cmdUpper = command;
        transform(cmdUpper.begin(), cmdUpper.end(), cmdUpper.begin(), ::toupper);


        if(cmdUpper == "EXIT"){
            break;
        }
        else if(cmdUpper.rfind("CREATE TABLE", 0)==0){
            cout<<"CREATE TABLE command detected"<<endl;
        }
        else if (cmdUpper.rfind("INSERT INTO", 0) == 0) {
            cout << "INSERT command detected" << endl;
        }
        else if (cmdUpper.rfind("SELECT", 0) == 0) {
            cout << "SELECT command detected" << endl;
        }
        else if (cmdUpper.rfind("DELETE", 0) == 0) {
            cout << "DELETE command detected" << endl;
        }
        else if (cmdUpper.rfind("UPDATE", 0) == 0) {
            cout << "UPDATE command detected" << endl;
        }
        else {
            cout << "Unknown command" << endl;
        }
    }
    cout<<"Exiting KittyDB..."<<endl;
}