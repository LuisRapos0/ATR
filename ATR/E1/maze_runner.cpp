#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <stdlib.h>

unsigned int num_rows, num_cols;
std::vector<std::vector<char>> Maze;
struct Position
{
  int row;
  int col;
};
std::stack<Position> valid_positions;

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


bool walk(Position pos)
{
  if(!valid_positions.empty())
    valid_positions.pop();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));


  if(Maze.at(pos.row).at(pos.col)=='s')
  {
    Maze.at(pos.row).at(pos.col)='o';
    system("clear");
    print_maze();
    return 1;
  }

  Maze.at(pos.row).at(pos.col)='o';
  system("clear");
  print_maze();

  if(is_valid_position(pos.row+1,pos.col))
  {
    Position x = {pos.row+1,pos.col};
    valid_positions.push(x);
  }
  if(is_valid_position(pos.row,pos.col+1))
  {
    Position x = {pos.row,pos.col+1};
    valid_positions.push(x);
  }
  if(is_valid_position(pos.row-1,pos.col))
  {
    Position x = {pos.row-1,pos.col};
    valid_positions.push(x);
  }
  if(is_valid_position(pos.row,pos.col-1))
  {
    Position x = {pos.row,pos.col-1};
    valid_positions.push(x);
  }
  
  if (valid_positions.empty())
    return 0;
  
  Maze.at(pos.row).at(pos.col)='.';
  bool exit_found = walk(valid_positions.top());
  return exit_found;
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
  
  bool exit_found = walk(start_pos);
  if (exit_found) {
      std::cout << "Saída encontrada!" << std::endl;
  } else {
      std::cout << "Não foi possível encontrar a saída." << std::endl;
  }

  return 0;
}