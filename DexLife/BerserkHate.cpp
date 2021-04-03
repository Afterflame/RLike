#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <random>
#include <functional>
#include <string>
#include <conio.h>

using namespace std;


#define KEY_UP    72
#define KEY_LEFT  75
#define KEY_RIGHT 77
#define KEY_DOWN  80

string spider[14] = {
"         (           ",
"          )          ",
"         (           ",
"   /\\  .-\"\"\"-.  /\\   ",
"  //\\\\/  ,,,  \\//\\\\  ",
"  |/\| ,;;;;;, |/\\|  ",
"  //\\\\\\;-\"\"\"-;///\\\\  ",
" //  \\/   .   \/  \\\\ ",
"(| ,-_| \\ | / |_-, |)",
"  //`__\\.-.-./__`\\\\  ",
" // /.-(() ())-.\\ \\\\ ",
"(\\ |)   '---'   (| /)",
" ` (|           |) ` ",
"   \\)           (/   "
};
string bat[7] = {
" /\\                 /\\ ",
"/ \\'._   (\\_/)   _.'/ \\",
"|.''._'--(o.o)--'_.''.|",
" \\_ / `;=/ \" \\=;` \\ _/ ",
"   `\\__| \\___/ |__/`   ",
"        \\(_|_)/        ",
"         \" ` \"         "
};
string gg[8] = {
"     .---.  ",
"   .-_:___+.",
"   |__ --==|",
"   [  ]  :[|",
"   |__| I=[|",
"   / / ____|",
"  |-/.____.+",
" /___\\ /___\\"
};
string attacc[4] = {
"  ^  ",
" | | ",
"_|_|_",
"  |  "
};
string deffense[4] = {
"_____",
"|   |",
"\\   /",
" \\_/ "
};

const int mapSize = 50;
const int screenSize = 23;
const int fog_dist = 11;
const float initial_wall_density = 0.4;
const float skip_coef = 0.1;
void clear() {
    system("cls");
    //std::cout << "\x1B[2J\x1B[H";
}
int sq(int a) {
    return a * a;
}
struct Coord
{
    int x, y;
    bool operator!=(const Coord other) {
        return(this->x != other.x || this->y != other.y);
    }
    bool operator==(const Coord other) {
        return(this->x == other.x && this->y == other.y);
    }
    Coord operator+(const Coord other) {
        return { this->x + other.x, this->y + other.y };
    }
};
class World;
class Character;
class Dice {
public:
    static int Roll(int max) {
        if (max <= 0) return 0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> RanDist(1, max);
        return RanDist(gen);
    }
    static Coord Dir() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> RanDist(0, 3);
        int dircode = RanDist(gen);
        switch (dircode) {
        case 0:
            return Coord{ 0,1 };
            break;
        case 1:
            return Coord{ 0,-1 };
            break;
        case 2:
            return Coord{ 1,0 };
            break;
        case 3:
            return Coord{ -1,0 };
            break;
        default:
            break;
        }
    }
};
class Ratt {
public:
    struct Action
    {
        bool isDef, isAtk;
        int defVal, atkVal;
    };
private:
    Coord coord = {-1-1};
    int hp = 0;
    int atk = 0;
    int def = 0;
    bool boss=0;
    Action nextAction = {0,0,0,0};
    World* world = nullptr;
    bool lazy = 0;
public:
    Ratt() {};
    Ratt(World* world, Coord coord = { -1,-1 }, bool isBoss=0);
    virtual void Act();
    void MoveTo(Coord coord);
    int GetHP() {
        return hp;
    }
    Coord GetPoint() {
        return coord;
    }
    const Action SetupAction() {
        int actionType = Dice::Roll(3);
        switch (actionType)
        {
        case 1:
            nextAction = Action{ 1,0,def*3,0 };
            break;
        case 2:
            nextAction = Action{ 0,1,0,atk*2 };
            break;
        case 3:
            nextAction = Action{ 1,1,def,atk };
            break;
        default:
            nextAction = Action{ 0,0,-1,-1 };
            break;
        }
        return nextAction;
    }
};

double dist(Coord a, Coord b) {
    return sqrt(sq(a.x - b.x) + sq(a.y - b.y));
}

class Map {
private:
    char matrix[mapSize][mapSize];
public:
    Map() {
        for (int i = 0; i < mapSize; i++) {
            for (int j = 0; j < mapSize; j++) {
                matrix[i][j] = '#';
            }
        }
    }
    char GetPoint(Coord coord) {
        return matrix[mapSize - coord.y - 1][coord.x];
    }
    void SetPoint(Coord coord, char c) {
        matrix[mapSize - coord.y - 1][coord.x] = c;
    }
};
class World {
private:
    Map map;
    Coord charcoord;
    Character* character;
    int difficulty = 2;
public:
    vector<Ratt*> ratts;
    int Difficulty() {
        return difficulty;
    }
    void DiffSet( int d) {
        difficulty = d;;
    }
    void DiffINC() {
        difficulty++;
    }
    World() {
        SetChar(genMap());
        for (int i = 0; i < mapSize * mapSize / 200; i++) {
            ratts.push_back(new Ratt(this));
        }
    }
    Coord GetRandomPoint() const
{
    Coord coord;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distWidth(1, mapSize-2), distHeight(1, mapSize-2);
    coord.x = distWidth(gen);
    coord.y = distHeight(gen);
    return coord;
}
    Coord genMap()
    {
        int iterations = mapSize * mapSize * (1- initial_wall_density);
        int coefForRepeat = 10/skip_coef;
        int direction;
        Coord lastPoint, startPoint, finishPoint;

        Coord randomPoint = GetRandomPoint();
        SetPoint(randomPoint, '.');
        lastPoint = finishPoint = startPoint = randomPoint;
        for (auto i = 0; i < iterations * coefForRepeat; i += coefForRepeat)
        {
            direction = Dice::Roll(4)-1;
            switch (direction)
            {
            case 0: // w
                if (lastPoint.y < mapSize - 3) {
                    if (GetPoint({ lastPoint.x, lastPoint.y + 1 }) == '.')
                        i -= coefForRepeat - 10;
                    SetPoint({ lastPoint.x, lastPoint.y + 1 }, '.');
                    lastPoint.y++;
                }
                else
                    i -= coefForRepeat;
                break;
            case 1: // a
                if (lastPoint.x > 1) {
                    if (GetPoint({ lastPoint.x - 1, lastPoint.y }) == '.')
                        i -= coefForRepeat - 10;
                    SetPoint({ lastPoint.x - 1, lastPoint.y }, '.');
                    lastPoint.x--;
                }
                else
                    i -= coefForRepeat;
                break;
            case 2: // s
                if (lastPoint.y > 1) {
                    if (GetPoint({ lastPoint.x, lastPoint.y - 1 }) == '.')
                        i -= coefForRepeat - 10;
                    SetPoint({ lastPoint.x, lastPoint.y - 1 }, '.');
                    lastPoint.y--;
                }
                else
                    i -= coefForRepeat;
                break;
            case 3: //d
                if (lastPoint.x < mapSize - 3) {
                    if (GetPoint({ lastPoint.x, lastPoint.y + 1 }) == '.')
                        i -= coefForRepeat - 10;
                    SetPoint({ lastPoint.x, lastPoint.y + 1 }, '.');
                    lastPoint.x += 1;
                }
                else
                    i -= coefForRepeat;
                break;
            default:
                std::cout << "Direction mismatch! Dungeon generation failed." << std::endl
                    << "Direction: " << direction << std::endl;
                return { -1, -1 };
            }
            if (lastPoint.y < startPoint.y) startPoint = lastPoint;
            if (lastPoint.y > finishPoint.y) finishPoint = lastPoint;
        }
        ratts.push_back(new Ratt(this, finishPoint, 1));
        SetPoint(Coord{ finishPoint.x, finishPoint.y + 1 }, 'E');
        return startPoint;
    }
    void AddChar(Character* character) {
        this->character = character;
    }
    bool isTransparent(char c) {
        if (c == 'M' || c == '#') return false;
        else return true;
    }
    bool isVisibleLine(Coord coord1, Coord coord2) {
        int xStep = 0, yStep = 0;
        if (coord2.y != coord1.y)
            yStep = (coord2.y - coord1.y) / abs(coord2.y - coord1.y);
        if (coord2.x != coord1.x)
            xStep = (coord2.x - coord1.x) / abs(coord2.x - coord1.x);
        if (abs(xStep) + abs(yStep) > 1) return 0;
        Coord curCheck = coord1;
        while (curCheck != coord2) {
            curCheck.x += xStep;
            curCheck.y += yStep;
            if (!isTransparent(map.GetPoint(curCheck))) return 0;
        }
        return 1;
    }
    double fracSpinner(double a, double b, int type) {
        if (type == 1)
            return a / b;
        else
            return b / a;
    }
    bool isVisible(Coord coord1, Coord coord2, int fogdist) {
        if (sqrt(sq(coord1.x - coord2.x) + sq(coord1.y - coord2.y)) >= fogdist) return false;
        return isVisibleWSpin(coord1, coord2, 1) || isVisibleWSpin(coord1, coord2, 2);
    }
    bool isVisibleWSpin(Coord coord1, Coord coord2, int type, double vector_ratio = 0) {
        if ((coord1.x == coord2.x) || (coord1.y == coord2.y))
            return isVisibleLine(coord1, coord2);
        if (vector_ratio == 0)
            vector_ratio = fracSpinner(static_cast<float>(coord1.x - coord2.x), static_cast<float>(coord1.y - coord2.y), type);
        int xStep = (coord2.x - coord1.x) / abs(coord2.x - coord1.x), yStep = (coord2.y - coord1.y) / abs(coord2.y - coord1.y);
        Coord curCheck = coord1;
        if (vector_ratio == fracSpinner(static_cast<float>(coord1.x - coord2.x), static_cast<float>(coord1.y - coord2.y), type)) {
            bool b1 = false, b2 = false;
            if (isTransparent(map.GetPoint({ curCheck.x, curCheck.y + yStep })))
                b1 = isVisibleWSpin({ curCheck.x, curCheck.y + yStep }, coord2, type, vector_ratio);
            if (isTransparent(map.GetPoint({ curCheck.x + xStep , curCheck.y })))
                b2 = isVisibleWSpin({ curCheck.x + xStep , curCheck.y }, coord2, type, vector_ratio);
            return b1 || b2;
        }
        if (!isTransparent(map.GetPoint(curCheck))) return 0;
        if ((curCheck.x == coord2.x) || (curCheck.y == coord2.y))
            return isVisibleLine(curCheck, coord2);
        while (curCheck != coord2) {
            double xStepRatio = fracSpinner(static_cast<float>(curCheck.x - coord2.x + xStep), static_cast<float>(curCheck.y - coord2.y), type);
            double yStepRatio = fracSpinner(static_cast<float>(curCheck.x - coord2.x), static_cast<float>(curCheck.y - coord2.y + yStep), type);
            if (abs(xStepRatio - vector_ratio) < abs(yStepRatio - vector_ratio))
                curCheck.x += xStep;
            else
                curCheck.y += yStep;
            if (!isTransparent(map.GetPoint(curCheck))) return 0;
            if ((curCheck.x == coord2.x) || (curCheck.y == coord2.y))
                return isVisibleLine(curCheck, coord2);
        }
        return 1;
    }
    bool AllowedToMove(Coord coord) {
        if (coord.x < 0 || coord.y < 0 || coord.x >= mapSize || coord.y >= mapSize) return false;
        if (!isTransparent(map.GetPoint(coord))) return 0;
        return true;
    }
    void Draw(int fog_distanse, Coord charcoord) {
        clear();
        Coord start = { charcoord.x- screenSize/2, charcoord.y- screenSize/2 };
        if (start.x + screenSize > mapSize) start.x = mapSize - screenSize;
        if (start.y + screenSize > mapSize) start.y = mapSize - screenSize;
        if (start.x < 0) start.x = 0;
        if (start.y < 0) start.y = 0;
        for (int y = start.y + screenSize-1; y >= start.y; y--) {
            for (int x = start.x; x < start.x + screenSize; x++) {
                if (isVisible({ x,y }, charcoord, fog_distanse))
                    cout << map.GetPoint({ x, y }) << ' ';
                else
                    cout << '=' << ' ';
            }
            cout << '\n';
        }
    }
    void SetChar(Coord coord) {
        charcoord = coord;
        SetPoint(charcoord, '@');
    }
    void MoveChar(Coord coord) {
        SetPoint(charcoord, '.');
        charcoord = coord;
        SetPoint(charcoord, '@');
    }
    void Update();
    void SetPoint(Coord coord, char c) {
        map.SetPoint(coord, c);
    }
    char GetPoint(Coord coord) {
        return map.GetPoint(coord);
    }
    const Coord GetCharCoord() {
        return charcoord;
    }
    void Act() {
        for (int i = 0; i < ratts.size(); i++) {
            ratts[i]->Act();
        }
    }
    Ratt* CheckFights() {
        for (int i = 0; i < ratts.size(); i++) {
            if (ratts[i]->GetPoint() == charcoord) {
                return ratts[i];
            }
        }
        return nullptr;
    }
};
class Character {
private:
    Coord coord = {-1,-1};
    int lvl = 5;
    int exp = 0;
    int energy = 3;
    World* world;
    int hp = 30;
    int maxhp = 30;
public:
    Character() = default;
    Character(Coord coord, World* world): world(world), coord(coord) {
        world->SetPoint({ coord.x,coord.y - 1 }, 'M');
    }
    Character& operator=(Character& other) {
        this->coord = move(other.coord);
        this->world = move(other.world);
        return *this;
    }
    void Input(char input) {
        switch (input)
        {
        case 'u':
        case 'd':
        case 'l':
        case 'r':
            this->Move(input);
            break;
        default:
            break;
        }
    }
    void Move(char dir) {
        switch (dir)
        {
        case ('u'):
            coord.y++;
            if (!world->AllowedToMove(coord))
                coord.y--;
            break;
        case ('d'):
            coord.y--;
            if (!world->AllowedToMove(coord))
                coord.y++;
            break;
        case ('l'):
            coord.x--;
            if (!world->AllowedToMove(coord))
                coord.x++;
            break;
        case ('r'):
            coord.x++;
            if (!world->AllowedToMove(coord))
                coord.x--;
            break;
        default:
            break;
        }
        if (world->GetPoint(coord) == 'E') {
            delete world;
            world = new World();
            world->AddChar(this);
            world->DiffSet(GetLVL() - 3);
            coord = (world->GetCharCoord());
            world->SetPoint({ coord.x,coord.y - 1 }, 'M');
            world->Update();
        }
        world->MoveChar(coord);
        cout << coord.x << ' ' << coord.y;
    }
    int GetLVL() {
        return lvl;
    }
    int GetEXP() {
        return exp;
    }
    int GetHP() {
        return hp;
    }
    int GetEnergy() {
        return energy;
    }
    bool TakeDMG(int dmg) {
        hp -= dmg;
        if (hp <= 0) {
            delete world;
            hp = maxhp;
            exp = 0;
            lvl = 5;
            world = new World();
            world->AddChar(this);
            world->DiffSet(GetLVL() - 3);
            coord = (world->GetCharCoord());
            world->SetPoint({ coord.x,coord.y - 1 }, 'M');
            world->Update();
            return 1;
        }
        return 0;
    }
    void GiveEXP(int ex) {
        exp += ex;
        while (exp / lvl > 0) {
            exp -=lvl;
            lvl++;
            world->DiffINC();
            if (maxhp / 2 < hp)
                hp = maxhp;
            else hp += maxhp / 2;
            maxhp += lvl;
            hp += lvl;
        }
    }
};
void addSprite(string field[], string sprite[], Coord pos, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < sprite[0].size(); x++) {
            field[pos.y + y][pos.x + x] = sprite[y][x];
        }
    }
}
void drawFightField(Character* character, Ratt::Action comming, int rattHP, int charHP, int charDef, int rattDef, int energy) {
    string field[30];
    for (int i = 0; i < 30; i++) {
        field[i].resize(80, ' ');
    }
    string AV = std::to_string(comming.atkVal);
    addSprite(field, gg, { 10,7 }, 8);
    addSprite(field, spider, { 50,3 }, 14);
    if (comming.isAtk) {
        addSprite(field, attacc, { 57 - (comming.isDef * 4),19 }, 4);
        addSprite(field, &AV, { 62 - (comming.isDef * 4),20 }, 1);
    }
    if (comming.isDef) {
        addSprite(field, deffense, { 58 + (comming.isAtk * 5),19 }, 4);
    }

    for (int i = 0; i < energy; i++) {
        string ener[1] = { "* "};
        addSprite(field, ener, { 15+i*2, 5 }, 1);
    }
    string RHP[2] = { "<3",std::to_string(rattHP) };
    addSprite(field, RHP, { 58,16 }, 2);
    string CHP[2] = { "<3",std::to_string(charHP) };
    addSprite(field, CHP, { 14,16 }, 2);

    string RDEF[2] = { "{>",std::to_string(rattDef) };
    addSprite(field, RDEF, { 62,16 }, 2);
    string CDEF[2] = { "{>",std::to_string(charDef) };
    addSprite(field, CDEF, { 18,16 }, 2);
    string AR[1] = { "->(" + std::to_string(character->GetLVL()) + ")" };
    string AL[1] = { "(" + std::to_string(character->GetLVL()) + ")<-" };
    addSprite(field, attacc, { 6,19 }, 4);
    addSprite(field, AL, { 11, 21 }, 1);
    addSprite(field, AR, { 17,21 }, 1);
    addSprite(field, deffense, { 23,19 }, 4);
    clear();
    for (int i = 0; i < 30; i++) {
        cout << field[i] << '\n';
    }
}
class Fight {
private:
    Ratt* ratt;
    Character* character;
public:
    Fight(Ratt* ratt, Character* character) : ratt(ratt), character(character) {};
    bool Execute() {
        int rattHP = ratt->GetHP();
        int charDef = 0;
        int rattDef = 0;
        int c = 0;
        while (1) {
            Ratt::Action comming = ratt->SetupAction();
            for (int energy = 0; energy < character->GetEnergy(); energy++) {
            tryAgain:
                drawFightField(character, comming, rattHP, character->GetHP(), charDef, rattDef, character->GetEnergy() - energy);
                c = _getch();
                if (c == 224 && _kbhit())
                    c = _getch();
                else
                    c = tolower(c);
                switch (c) {
                case 'a':
                case KEY_LEFT:
                    rattDef -= character->GetLVL();
                    if (rattDef < 0) {
                        rattHP += rattDef;
                        rattDef = 0;
                    }
                    if (rattHP <= 0) {
                        character->GiveEXP(3);
                        return true;
                    }
                    break;
                case 'd':
                case KEY_RIGHT:
                    charDef += character->GetLVL();
                    break;
                default:
                    goto tryAgain;
                    break;
                }
            }
            if (comming.isAtk) {
                charDef -= comming.atkVal;
                if (charDef < 0)
                    if (character->TakeDMG(-charDef)) return 0;
            }
            if (comming.isDef) {
                rattDef = comming.defVal;
            }
            charDef = 0;
        }
    }
};
class Game {
private:
    World* world;
    Character character;
public:
    Game() {
        world = new World();
        character = *new Character(world->GetCharCoord(), world);
        world->AddChar(&character);
        world->Update();
    }
    bool CheckFights() {
        auto ratt = world->CheckFights();
        if (ratt) {
            auto fight = Fight(ratt, &character);
            fight.Execute();
            world->Update();
            for (int i = 0; i < world->ratts.size(); i++) {
                if (world->ratts[i] == ratt) {
                    world->ratts.erase(world->ratts.begin() + i);
                    return true;
                }
            }
        }
        return true;
    }
    void Starto() {
        int c = 0;
        world->Update();
        while (1)
        {
            c = _getch();
            if (c == 224 && _kbhit())
                c = _getch();
            else
                c = tolower(c);
            switch (c) {
            case 'w':
            case KEY_UP:
                character.Input('u');
                break;
            case 's':
            case KEY_DOWN:
                character.Input('d');
                break;
            case 'a':
            case KEY_LEFT:
                character.Input('l');
                break;
            case 'd':
            case KEY_RIGHT:
                character.Input('r');
                break;
            case ' ':
                break;
            default:
                cout << endl << (char)c << endl;
                break;
            }
            if (!CheckFights()) return;
            world->Act();
            if (!CheckFights()) return;
            world->Update();
        }
    }
};
void World::Update() {
    this->Draw(fog_dist, charcoord);
    cout << "LV:" << character->GetLVL() << " EX:" << character->GetEXP();
    cout << " HP:" << character->GetHP() << " DIF:" << Difficulty();
}
Ratt::Ratt(World* world, Coord coord, bool isBoss){
    boss = isBoss;
    this->world = world;
    this->coord = coord;
    if (this->coord == Coord{ -1,-1 })
        while (world->GetPoint(this->coord) != '.' || world->isVisible(this->coord, world->GetCharCoord(), fog_dist))
            this->coord = world->GetRandomPoint();
    world->SetPoint(this->coord, '6'+boss);
    atk = world->Difficulty() * 2 + Dice::Roll(world->Difficulty()) + 1+boss*4;
    hp = (3+5*boss)*sq(world->Difficulty()) + Dice::Roll((2+3*boss)*sq(world->Difficulty())) + Dice::Roll(5+(boss*10));
    def = world->Difficulty() + Dice::Roll(world->Difficulty() / (3-boss*2))+1+boss;
}
void Ratt::Act() {
    if (boss) return;
    if (world->isVisible(coord, world->GetCharCoord(), fog_dist * 1.3)) {
        if (lazy) {
            if (Dice::Roll(20) > 5)
                lazy = 0;
            return;
        }
        if (Dice::Roll(20) > 4) {
            Coord dir = Dice::Dir();
            for(int c = 0; c < 6; c++){
                dir = Dice::Dir();
                while(!world->AllowedToMove(coord + dir))
                    dir = Dice::Dir();
                if (dist(world->GetCharCoord(), coord + dir) < dist(world->GetCharCoord(), coord)) {
                    MoveTo(coord +dir);
                    if (Dice::Roll(20) < 5)
                        lazy = 1;
                    return;
                }
            }
            MoveTo(coord + dir);
            return;
        }
        else {
            Coord dir = Dice::Dir();
            while (!world->AllowedToMove(coord + dir)) {
                dir=Dice::Dir();
            }
            MoveTo(coord + dir);
            lazy = 1;
            return;
        }
    }
    else {
        if (lazy) {
            if (Dice::Roll(20) > 12)
                lazy = 0;
            return;
        }
        int i = 0, j = 0;
        while ((abs(i) + abs(j) != 1) &&
            world->AllowedToMove(coord + Coord{ i,j })) {
            i = Dice::Roll(3) - 2;
            j = Dice::Roll(3) - 2;
        }
        MoveTo(coord + Coord{ i,j });
        lazy = 1;
        return;
    }
}
void Ratt::MoveTo(Coord coord) {
    world->SetPoint(this->coord, '.');
    world->SetPoint(coord, '6');
    this->coord = coord;
}
int main()
{
    Game game = Game();
    game.Starto();
}
