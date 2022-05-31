#include "fairy_tail.hpp"
#include <unistd.h>

#include <vector>

using pos = std::pair<int, int>;
using map = std::vector<std::vector<char>>;
using map1 = std::vector<std::vector<int>>;

struct Player{
    Player(Character character, Direction mDir, map &map, map1 &map1, bool l) : name(character), dir(mDir), mainDir(mDir), map(map), map1(map1), leftTurn(l) {}
    Character name;
    int x = 10, y = 10;
    map &map;
    map1 &map1;
    Direction dir, mainDir;
    bool leftTurn;  // движение против часовой стрелке / по часовой
    bool stuck = false;
};

void printMap(map &map){
    for (int i=19; i>-1; i--){
        for (int j=0; j<20; j++){
            std::cout << map[i][j] << '\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

int f(char ch){
    switch (ch) {
        case '?':
            return 0;
        case '#':
            return 1;
        case '.':
            return 1;
        case '@':
            return 2;
        case '&':
            return 2;
        case 'E':
            return 1;
        case 'I':
            return 1;
        default:
            std::cerr << "error. not known char";
            return -1;
    }
}

Direction operator!(Direction direction){
    switch (direction) {
        case Direction::Up:
            return Direction::Down;
        case Direction::Down:
            return Direction::Up;
        case Direction::Left:
            return Direction::Right;
        case Direction::Right:
            return Direction::Left;
        case Direction::Pass:
            return Direction::Pass;
    }
}

int attempt = 0;

void MapUnite(Player &p1, Player &p2){
    //Работает хорошо, если оба игрока в одной точке, иначе может быть неточной
    std::vector<std::vector<char > > map(20, std::vector<char >(20, '?'));
    for (int x = 0; x < 20; x++){
        for (int y = 0; y < 20; y++) {
            char ch1 = ((x + p1.x - 10 > -1) && (x + p1.x - 10 < 20) && (y + p1.y - 10 > -1) && (y + p1.y - 10 < 20)) ? p1.map[x + p1.x - 10][y + p1.y - 10] : '?';
            char ch2 = ((x + p2.x - 10 > -1) && (x + p2.x - 10 < 20) && (y + p2.y - 10 > -1) && (y + p2.y - 10 < 20)) ? p2.map[x + p2.x - 10][y + p2.y - 10] : '?';

            if ((ch1 == '#' && ch2 == '.') || (ch2 == '#' && ch1 == '.')){
                if (attempt == 0) {
                    attempt++;
                    p1.y++;
                    MapUnite(p1, p2);
                    return;
                }
                if (attempt == 1) {
                    p1.y -= 2;
                    MapUnite(p1, p2);
                    return;
                }
                return;
            }

            map[x][y] = (f(ch1) > f(ch2)) ? ch1 : ch2;
        }
    }

    std::cout << "Final Map" << std::endl;
    printMap(map);
}

//Turn 90 degrees
void turn90(Player &player, bool leftTurn) {
    switch (player.dir) {
        case Direction::Down:
            player.dir = (leftTurn) ? Direction::Right : Direction::Left;
            break;
        case Direction::Right:
            player.dir = (leftTurn) ? Direction::Up : Direction::Down;
            break;
        case Direction::Up:
            player.dir = (leftTurn) ? Direction::Left : Direction::Right;
            break;
        case Direction::Left:
            player.dir = (leftTurn) ? Direction::Down : Direction::Up;
            break;
        default:
            player.dir = Direction::Pass;
            break;
    }
}

//Get vector of direction
pos vec(Direction direction){
    switch (direction) {
        case Direction::Up:
            return {1,0};
        case Direction::Down:
            return {-1, 0};
        case Direction::Left:
            return {0, -1};
        case Direction::Right:
            return {0, 1};
        case Direction::Pass:
            return {0,0};
    }
}

bool checkBoundaries(Player &player){
    return ((player.x + vec(player.dir).first > -1) && (player.x + vec(player.dir).first < 19) &&
            (player.y + vec(player.dir).first > -1) && (player.y + vec(player.dir).second < 19));
}

bool hasBeenThere(Player &player){
    return player.map1[player.x + vec(player.dir).first][player.y + vec(player.dir).second] > 0;
}

void step(Player &player, Fairyland &world){

    if (player.stuck) {
        player.dir = Direction::Pass;
        return;
    }

    Direction BackDir = !player.dir;
    int counter = 0;

    if (player.dir != player.mainDir)
        turn90(player, player.leftTurn);

    if (checkBoundaries(player))
        if (player.map1[player.x + vec(player.dir).first][player.y + vec(player.dir).second] > 0)
            turn90(player, player.leftTurn);

    while (!world.canGo(player.name, player.dir) || player.dir == BackDir || hasBeenThere(player)) {

        if (checkBoundaries(player) && !hasBeenThere(player) && !world.canGo(player.name, player.dir))
            player.map[player.x + vec(player.dir).first][player.y + vec(player.dir).second] = '#';
        turn90(player, !player.leftTurn);

        //Случай, если некуда идти //Окружен со всех сторон
        if ((counter > 6) && player.dir == BackDir && !world.canGo(player.name, player.dir)) {
            player.stuck = true;
            player.dir = Direction::Pass;
            break;
        }

        if ((counter > 6) && world.canGo(player.name, player.dir))
            break;

        counter++;
    }

    player.x += vec(player.dir).first;
    player.y += vec(player.dir).second;
    player.map1[player.x][player.y]++;

    if (player.map[player.x][player.y] == '?')
        player.map[player.x][player.y] = '.';

    if (player.map1[player.x][player.y] > 3)
        player.stuck = true;

    printMap(player.map);
}

int walk(){
    //initialize players
    Fairyland world;
    std::vector<std::vector<char > > map1(20, std::vector<char >(20, '?'));
    std::vector<std::vector<char > > map2(20, std::vector<char >(20, '?'));

    std::vector<std::vector<int > > map11(20, std::vector<int >(20, 0));
    std::vector<std::vector<int > > map22(20, std::vector<int >(20, 0));

    map1[10][10] = '@';
    map2[10][10] = '&';

    map11[10][10] = 1;
    map22[10][10] = 1;

    std::vector<Player> players = {{Character::Ivan, Direction::Left, map1, map11, true},
                                   {Character::Elena, Direction::Right, map2, map22, false}};

    do {
        for (auto &player: players){
            step(player, world);
            //unsigned int microsecond = 1000000;
            //usleep(1 * microsecond);//sleeps for 3 second
        }
        if (players[0].stuck && players[1].stuck){
            std::cout << "Players are stuck. No solution exists!" << std::endl;
            break;
        }
    } while (!world.go(players[0].dir, players[1].dir));

    players[0].map[players[0].x][players[0].y] = 'I';
    players[1].map[players[1].x][players[1].y] = 'E';

    printMap(players[0].map);
    printMap(players[1].map);

    MapUnite(players[0], players[1]);
    return world.getTurnCount();
}

void MakeRandomMap(){
    srand(time(nullptr));
    std::ofstream fout("input.txt");
    int ivan_x = rand() % 10, ivan_y = rand() % 10;
    int elena_x = rand() % 10, elena_y = rand() % 10;
    for (int i=0; i<10; i++) {
        for (int j = 0; j < 10; j++) {
            if ((i == ivan_x) && (j == ivan_y)) {
                fout << '@' << '\t';
                continue;
            }
            if ((i == elena_x) && (j == elena_y)) {
                fout << '&' << '\t';
                continue;
            }
            if (random() % 10 < 7) //random() % 2
                fout << '.' << '\t';
            else
                fout << '#' << '\t';
        }
        fout << std::endl;
    }

}

int main()
{
    MakeRandomMap();
    if (const int turns = walk()) {
        std::cout << "Found in " << turns << " turns" << std::endl;
        if (turns > 250)
            std::cout << "ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR";
    }
    else
        std::cout << "Not found" << std::endl;

    return 0;
}