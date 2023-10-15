#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

enum class Direction {
  RIGHT,
  LEFT,
  STAY
};

enum class Error {
  NO_ERROR,
  OPENING_FILE,
  INCORRECT_DIRECTION,
  INCORRECT_STATE,
  NO_INSTRUCTION_FOR_LETTER
};

class TuringMachine {
private:
  bool stop, stepByStep;
  int pointer;
  mutable Error err;
  std::string line, curState, initState;
  std::string inputFile, instructFile, outputFile;
  std::map<std::string, std::vector<std::string>> commands; 
  
  void readLine();
  void readInstructions();
  void print() const;
  void printRes(std::ostream &f) const;
  void printPointer(std::ostream &f) const;

  std::string findLine(const std::string &key) const;
  std::string get(const std::string &cmd, int pos) const;
  char getReadLetter(const std::string &cmd) const;
  char getWriteLetter(const std::string &cmd) const;
  Direction getDirection(const std::string &cmd);
  std::string getNextState(const std::string &cmd);
  void writeLetter(const std::string &cmd);
  inline void changeState(const std::string &state);
  void move(const std::string &cmd);
  void moveLeft();
  void moveRight();

public:
  TuringMachine();
  TuringMachine(const std::string &inp, const std::string &inst, const std::string &oup, const std::string &init_state, bool step);
  ~TuringMachine() = default;
  Error run();
};

TuringMachine::TuringMachine() :
  inputFile("input.txt"), instructFile("instructions.txt"), outputFile("output.txt"),
  pointer(0), initState("smallest_key"), err(Error::NO_ERROR), stop(false), stepByStep(false)
  {}

TuringMachine::TuringMachine(const std::string &inp, const std::string &inst, const std::string &oup, const std::string &init_state, bool step) :
  inputFile(inp), instructFile(inst), outputFile(oup), initState(init_state), pointer(0), err(Error::NO_ERROR), stop(false), stepByStep(step)
  {}

void TuringMachine::move(const std::string &cmd) {
  Direction dir = getDirection(cmd);
  if (dir == Direction::RIGHT)
    moveRight();
  else if (dir == Direction::LEFT)
    moveLeft();
}

void TuringMachine::moveLeft() {
  if (pointer > 0)
    pointer--;
  else
    line.insert(line.begin(), ' ');
}

void TuringMachine::moveRight() {
  if (pointer < line.size() - 1)
    pointer++;
  else {
    line.push_back(' ');
    pointer++;
  }
}

void TuringMachine::changeState(const std::string &state) {
  curState = state;
}

void TuringMachine::writeLetter(const std::string &cmd) {
  char c = getWriteLetter(cmd);
  if (c != '*')
    line[pointer] = c;
}

void TuringMachine::readLine() {
  std::ifstream in(inputFile);
  if (in.is_open())
    std::getline(in, line);
  else
    err = Error::OPENING_FILE;
}

void TuringMachine::readInstructions() {
  std::string tmp, cur, key;
  std::ifstream in(instructFile);
  if (in.is_open()) {
    while(std::getline(in, tmp) && in) {
      if (tmp[0] == ';')
        continue;
      else {
        std::stringstream s(tmp);
        if (!(s >> key))
          continue;
        std::getline(s, cur);
        commands[key].push_back(cur);
        }
    }
  }
  else
    err = Error::OPENING_FILE;
}

void TuringMachine::printRes(std::ostream &f) const {
  f << "State: " << curState << std::endl << std::setw(20) << line << std::endl;
  printPointer(f);
  f << std::endl;
}

void TuringMachine::print() const {
  if (outputFile == "console") {
    if (stepByStep)
      std::cin.get();
    printRes(std::cout);
  } else {
    std::ofstream f(outputFile, std::ios::app);
    if (f.is_open())
      printRes(f);
    else
      err = Error::OPENING_FILE;
  }
}

void TuringMachine::printPointer(std::ostream &f) const {
  f << std::setw(20 - line.size() + 1);
  for (size_t i = 0; i < line.size(); ++i) {
    if (i < pointer)
      f << ' ';
    else {
      f << '^';
      break;
    }
  }
}

std::string TuringMachine::findLine(const std::string &key) const {
  std::string default_cmd;
  char curEl = line[pointer];
  for (const std::string &s : commands.at(key)) {
    char c = getReadLetter(s);
    if (c != '*' && c == curEl)
      return s;
    if (c == '*')
      default_cmd = s;
  }

  if (!default_cmd.empty())
    return default_cmd;

  err = Error::NO_INSTRUCTION_FOR_LETTER;
  return 0;
}

std::string TuringMachine::get(const std::string &cmd, int pos) const {
  int n = -1;
  std::string letter;
  std::stringstream s(cmd);
  while (pos != n && s >> letter) {
    n++;
  }
  return letter;
}

char TuringMachine::getReadLetter(const std::string &cmd) const {
  char s = get(cmd, 0)[0];
  if (s == '_')
    return ' ';
  return s;
}

char TuringMachine::getWriteLetter(const std::string &cmd) const {
  char s = get(cmd, 1)[0];
  if (s == '_')
    return ' ';
  return s;
}

Direction TuringMachine::getDirection(const std::string &cmd) {
  char s = get(cmd, 2)[0];
  switch(s) {
    case 'r':
      return Direction::RIGHT;
    case 'l':
      return Direction::LEFT;
    case '*':
      return Direction::STAY;
  }
  err = Error::INCORRECT_DIRECTION;
  return Direction::STAY;
}

std::string TuringMachine::getNextState(const std::string &cmd) {
  std::string s = get(cmd, 3);
  if (s.substr(0, 4) == "halt")
    stop = true;
  else if (commands.count(s) == 0)
    err = Error::INCORRECT_STATE;
  return s;
}

Error TuringMachine::run() {
  readLine();
  readInstructions();
  if (err != Error::NO_ERROR)
    return err;

  std::string curLine;
  std::string state = (initState == "smallest_key") ? commands.begin()->first : initState;

  while(err == Error::NO_ERROR) {
    changeState(state);
    print();
    if (stop)
      break;
    curLine = findLine(curState);
    if (err == Error::NO_INSTRUCTION_FOR_LETTER)
      break;
    writeLetter(curLine);
    move(curLine);
    state = getNextState(curLine); // add error check here
  }

  return err;
}

void enumToString(const Error &err) {
  switch(err) {
  case Error::NO_ERROR:
    std::cout << "everything works great!\n";
    break;
  case Error::INCORRECT_DIRECTION:
    std::cout << "incorrect direction in instructions.\n";
    break;
  case Error::INCORRECT_STATE:
    std::cout << "incorrect next state in instructions.\n";
    break;
  case Error::NO_INSTRUCTION_FOR_LETTER:
    std::cout << "no instruction for current letter.\n";
    break;
  case Error::OPENING_FILE:
    std::cout << "can not open file. Please, check filepaths.\n";
    break;
  default:
    std::cout << "everything works great!\n";
  }
}

int main(int argc, char** argv) {
  std::string output = "console";
  std::string init_state = "smallest_key";
  bool step_by_step = false;

  if (argc < 3) {
    std::cout << "Usage: " << argv[0] 
    << "input_line.txt instructions.txt output.txt(default: console) init_state(default: smallest_key) step_by_step(default: false, only in console mode)\n";
    return 1;
  }
  if (argc > 3) {
    output = argv[3];
    if (argc > 4) {
    init_state = argv[4];
    if (argc > 5)
      step_by_step = argv[5];
    }
  }

  TuringMachine machine(argv[1], argv[2], output, init_state, step_by_step);
  Error err = Error::NO_ERROR;
  err = machine.run();
  if (err != Error::NO_ERROR) {
    std::cout << "Error occured: ";
    enumToString(err);
    std::cout << std::endl;
    return 2;
  }

  return 0;
}
