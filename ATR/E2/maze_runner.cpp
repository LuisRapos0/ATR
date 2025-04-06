#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <algorithm>

unsigned int num_rows, num_cols;
bool stop_printing = false;
std::vector<std::vector<char>> Maze;
struct Position
{
  int row;
  int col;
};
bool exit_found = false;

bool is_valid_position(int row, int col) {
  if(row >= 0 && row < num_rows && num_cols >= 0 && col < num_cols)
  {
    if(Maze.at(row).at(col) == 'x' || Maze.at(row).at(col) == 's')
      return 1;
    else return 0;
  }
  else
    return 0;
}

void print_maze()
{
  for(int i = 0; i < num_rows; i++)
  {
    for(int j = 0; j < num_cols; j++)
    {
      if(Maze.at(i).at(j)=='o')
      {
        std::cout << "\033[1;31mo\033[0m";
        continue;
      }
      if(Maze.at(i).at(j)=='.')
      {
        std::cout << "\033[31m.\033[0m";
        continue;
      }
      if(Maze.at(i).at(j)=='x')
      {
        std::cout << "\033[33mx\033[0m";
        continue;
      }
      if(Maze.at(i).at(j)=='#')
      {
        std::cout << "#";
        continue;
      }
      if(Maze.at(i).at(j)=='s')
      {
        std::cout << "\033[32ms\033[0m";
        continue;
      }
      if(Maze.at(i).at(j)=='e')
      {
        std::cout << "\033[32me\033[0m";
        continue;
      }
    }
    std::cout << "\n";
  }
}
void maze_printer(){
  while(!stop_printing){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    system("clear");
    print_maze();
  }
}

Position load_maze(const std::string file_name)
{
  std::ifstream map_file(file_name);
  if (!map_file)
  {
    std::cout << "Erro ao abrir o arquivo do labirinto\n";
    std::cout << "Verifique o nome do labirinto inserido\n";
    std::cout << "Exemplo de execução: './maze_runner maze4'\n";
    return {-1,-1};
  }

  map_file >> num_rows >> num_cols;
  map_file.ignore(256, '\n');

  Maze.resize(num_rows);
  for(int i = 0; i < num_rows; i++)
  {
    Maze.at(i).resize(num_cols);
  }

  char ch;
  Position start_pos = {-1,-1};

  for(int i = 0; i < num_rows; i++)
  {
    for(int j = 0; j < num_cols; j++)
    {
      map_file >> std::noskipws >> ch;
      Maze.at(i).at(j) = ch;
      if (ch == 'e')
      {
        start_pos = {i,j};
      }
    }
    map_file.ignore(256, '\n');
  }

  if (start_pos.col == -1 && start_pos.row == -1)
  {
    std::cout << "Erro ao abrir o arquivo do labirinto\n";
    std::cout << "Posição inicial não encontrada\n";
    return {-1,-1};
  }

  return start_pos;
}

void do_join(std::thread& t)
{
    t.join();
}
void join_all(std::vector<std::thread>& v)
{
  std::for_each(v.begin(),v.end(),do_join);
}

void walk(Position pos)
{
  if(Maze.at(pos.row).at(pos.col)=='s')
  {
    Maze.at(pos.row).at(pos.col)='o';
    system("clear");
    print_maze();
    exit_found = true;
    return;
  }
  Maze.at(pos.row).at(pos.col)='o';
  
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  
  Position up_pos    = {pos.row+1,pos.col};
  Position right_pos = {pos.row,pos.col+1};
  Position down_pos  = {pos.row-1,pos.col};
  Position left_pos  = {pos.row,pos.col-1};

  bool up_valid    = is_valid_position(pos.row+1,pos.col);
  bool right_valid = is_valid_position(pos.row,pos.col+1);
  bool down_valid  = is_valid_position(pos.row-1,pos.col);
  bool left_valid  = is_valid_position(pos.row,pos.col-1);

  std::vector<std::thread> threads;
  if(up_valid)
    threads.push_back(std::thread(walk,up_pos));
  if(right_valid)
    threads.push_back(std::thread(walk,right_pos));
  if(down_valid)
    threads.push_back(std::thread(walk,down_pos));
  if(left_valid)
    threads.push_back(std::thread(walk,left_pos));

  Maze.at(pos.row).at(pos.col)='.';
  join_all(threads);

  return;
} 

int main(int argc, char **argv)
{
  system("clear");
  if (argc != 2)
  {
    std::cout << "Programa executado com o número de Parâmetros errado.\n";
    std::cout << "Execute no formato: maze_runner nome_do_labirinto\n";
    std::cout << "Exemplo de execução: './maze_runner maze4'\n";
    return 1;
  }

  std::string file_name = "data/";
  file_name.append(argv[1]);
  file_name.append(".txt");

  Position start_pos = load_maze(file_name);
  if (start_pos.col == -1 && start_pos.row == -1)
  return 1;

  print_maze();
  std::cout << "Pressione enter para iniciar";
  getchar();
  
  std::thread maze_print_thread(maze_printer);
  std::thread maze_runner_thread(walk,start_pos);

  maze_runner_thread.join();
  stop_printing = true;
  maze_print_thread.join();
  system("clear");
  print_maze();

  if (exit_found) {
      std::cout << "Saída encontrada!" << std::endl;
  } else {
      std::cout << "Não foi possível encontrar a saída." << std::endl;
  }

  return 0;
}