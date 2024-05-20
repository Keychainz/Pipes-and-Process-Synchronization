#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_SIZE 10

using namespace std;

//Solves inline math equation
string evaluateExpression(string expression)
{
  istringstream iss(expression);

  int result;
  iss >> result;

  char operation;
  int operand;

  while (iss >> operation >> operand) 
  {
      switch (operation) 
      {
        case '+':
          result += operand;
          break;
        case '-':
          result -= operand;
          break;
        case '*':
          result *= operand;
          break;
        case '/':
          result /= operand;
          break;
        default:
        cout << "Invalid operation: " << operation << std::endl;
        return "evaluateExpression ERROR!"; // or handle the error as needed
      }
    }
  return to_string(result);
}

//Checks if string contains more than 1 word
bool hasMultipleWords(string input)
{
  istringstream iss(input);
  int wordCount = 0;
  string word;

  while (iss >> word) 
  {
    wordCount++;
    if (wordCount > 1) 
    {
      return true;
    }
  }
  return false;
}

//Inputs variables into vector
void InputReader(string input, vector<string> &inputVector)
{
  string tempString = "";
  for(int i=0; i < input.size(); i++)
  {
    if(input[i] == ',' || input[i] == ' ' || input[i] == ';')
    {
      if(!tempString.empty())
      {
        inputVector.push_back(tempString);
      }
      tempString.clear();
      continue;
    }
    tempString = tempString + input[i];
  }
  if(!tempString.empty())
  {
    inputVector.push_back(tempString);
  }
}

//Loads keys into map and maps them to values if applicable
int InputToMap(string keys, string values, map<string,string> &variableMap)
{
  vector<string>inputValues;
  if(values != "")
  {
    InputReader(values, inputValues);
  }

  string variable = "";
  int n = 0;
  for(int i=0; i < keys.size(); i++)
  {
    if(keys[i] == ',' || keys[i] == ' ' || keys[i] == ';')
    {
      if(!variable.empty())
      {
        if(values != "")
        {
          variableMap[variable] = inputValues[n];
          n++;
        }
        else
        {
          variableMap[variable] = "";
          n++;
        }
      }
        variable.clear();
      continue;
    }
      variable = variable + keys[i];
  }
  return n;
}

//pipe read function, only needs pipe input
string read(pair<int,int>pipe)
{
  //close(pipe.second);
  int n;
  if (read(pipe.first, &n, sizeof(int)) < 0)
  {
    cout << "ERROR READING STRING SIZE!" << endl;
    //return;
  }

  char buffer[n + 1];
  if (read(pipe.first, buffer, n) < 0) 
  {
    cout << "ERROR READING STRING VALUE!" << endl;
    //return;
  }
  buffer[n] = '\0';  // Null-terminate the string
  string receivedString(buffer);
  return receivedString;
}

//pipe write function, needs pipe input and string to pipe
void write(pair<int,int>pipe, string value)
{
  //close(pipe.first);
  int n = value.length();
  if(write(pipe.second, &n, sizeof(int)) < 0)
  {
    cout << "ERROR WRITING STRING SIZE!" << endl;
    //return;
  }
  if(write(pipe.second, value.c_str(), n) < 0)
  {
    cout << "ERROR WRITING STRING VALUE!" << endl;
    //return;
  }
}

int main(int argc, char* argv[]) 
{
// Part 1
// Load variables into map, and map values to the variables. 
//If no values to map, variable will be mapped to empty string
/*______________________________________________________________________________________*/

  //create vector to hold pipe commands. And also a map to map variables to values
  vector<string> pipelines;
  map<string, string> variableMap;

  //Open both files, starting with pipeline file, then value file
  ifstream sFile(argv[1]);
  ifstream infile(argv[2]);
  //ifstream sFile("s1.txt");
  //ifstream infile("input1.txt");

  //Error handling for if files dont open
  if (!sFile.is_open()) 
  {
      cout << "Failed to open input file: " << argv[1] << endl;
      return EXIT_FAILURE;
  }
  if (!infile.is_open()) 
  {
      cout << "Failed to open input file: " << argv[2] << endl;
      return EXIT_FAILURE;
  }

  string keys = "";
  string values = "";

  //read input_var and input values;
  getline(sFile, keys);
  getline(infile, values);
  keys = keys.substr(10);

  //map values to input_var
  InputToMap(keys, values, variableMap);
  values.clear();
  keys.clear();


  //read second line in pipe command file
  getline(sFile, keys);
  keys = keys.substr(13);

  //input internal_var into map and assign them to ""
  int numPipes = InputToMap(keys, values, variableMap);
  values.clear();
  keys.clear();

  //load pipelines into vector, so each line in the s file will go into string vector
  while(getline(sFile, values))
    {
      while(values.back() == '\n'||values.back() == '\r'||values.back() == ' '||values.back() == ';')
        {
          values.pop_back();
        }
      pipelines.push_back(values);
    }
  sFile.close();
  infile.close();

// Part 2
// 
/*______________________________________________________________________________________*/

  vector<pair<int, int>>pipes; //Vector to store file descriptors for each pipe

  for (int i = 0; i < numPipes; ++i) //Create pipes dynamically
  {
    int pipefd[2];
    if (pipe(pipefd) == -1) //error checking
    {
      cout << "FAILED TO CREATE " << i << " PIPE!" << endl;
      exit(EXIT_FAILURE);
    }
    pipes.push_back({pipefd[0], pipefd[1]});
  }

  //Creating multiple processes from the parent using fork(). Using int pid to keep track of child processes. 
  //Using int pid to keep track of child processes. So the first child pid == 0, second child pid == 1 and ect.
  //Since there will be at most 10 variables, I identified the main process with a pid == 10.
  int pid;
  for(int i=0;i<numPipes;i++)
  {
    if(fork() == 0)
    {
      pid = i;
      break;
    }
    else
    {
      pid = MAX_SIZE;
    }
  }
  for(int i=0;i<numPipes;i++)
  {
    //wait(NULL);
  }
  
  //Loop through and iterate through the pipeline commands
  for (int i = 0; i < pipelines.size()-1; i++)
  { 
    string temp, value, sourceID, targetID;
    istringstream iss(pipelines[i]);
    
    while ((iss >> temp) && (temp != "->")) //loop to disect pipe command string
    {
      if(temp == "+"||temp == "-"||temp == "*"||temp == "/") // Grabs operator symbol
      {
        value = value + temp + " ";
      }
      else if(temp[0] == 'p') //Grabs Source ID if its an internal variable
      {
        sourceID = temp;
        value = value + temp + " ";
      }
      else //Grabs value of Source ID if a input variable
      {
        auto it = variableMap.find(temp);
        value = value + it->second + " ";
      }
    }
    iss >> targetID;
    
    if(pid < MAX_SIZE && pid == targetID.back() - '0') //If child, write to parent
    { 
      //cout << "child process " << pid << " wrote " << value << "to " << targetID << endl;
      write(pipes[targetID.back() - '0'], value);
    }
    else if(pid >= MAX_SIZE) //If Parent, read to map
    { 
      bool internalVar = false;
      string receivedString = read(pipes[targetID.back() - '0']);
      string tempHolder = "";
      stringstream iss(receivedString);
      while(iss >> temp)
      {
        if(temp[0] == 'p')
        {
          internalVar = true;
          if(isspace(variableMap[temp].back()))
          {
            variableMap[temp].pop_back();
          }
          variableMap[temp] = evaluateExpression(variableMap[temp]); //solve expression before passing value
          tempHolder += variableMap[temp] + " ";
        }
        else
        {
          tempHolder = tempHolder + temp + " ";
        }
      }
      if(internalVar)
      {
        receivedString = tempHolder;
      }
      variableMap[targetID] += receivedString;
      //cout << "Parent process read " << receivedString << "from " << targetID << endl;
    }
  }
  
  if(pid < MAX_SIZE) // End all child processes
  {
    exit(0);
  }

  // Close pipes when done using them
  for (int i = 0; i < numPipes; ++i) 
  {
    close(pipes[i].first);
    close(pipes[i].second);
  }

  //Solve any unsolved expressions in the map
  for (auto& entry : variableMap) 
  {
    if (hasMultipleWords(entry.second))
    {
      entry.second = evaluateExpression(entry.second);
    }
  }

  ofstream outfile("output.txt");
  outfile << "State of the map:" << endl;
  for (const auto& entry : variableMap) 
  {
    if(entry.first[0] == 'p')
    {
      if(entry.second == "")
      {
        outfile << entry.first << " -> " << "empty" << endl;
        continue;
      }
      outfile << entry.first << " -> " << entry.second << endl;
    }
  }
  outfile.close();
}