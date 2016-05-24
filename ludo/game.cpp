#include "game.h"
#define DEBUG 0

game::game(){
    game_delay = 1000;
    game_complete = false;
    turn_complete = true;
    for(int i = 0; i < 16; ++i){
         player_positions.push_back(-1);
    }
    color = 3;
}

void game::reset(){
    game_complete = false;
    turn_complete = true;
    for(auto i : player_positions){
        i = -1;
    }
    color = 3;
}


int game::rel_to_fixed(int relative_piece_index){
    return relative_piece_index + color * 4;
}

int game::isStar(int index){
    if(index == 5  ||
       index == 18 ||
       index == 31 ||
       index == 44){
        return 6;
    } else if(index == 11 ||
              index == 24 ||
              index == 37 ||
              index == 50){
        return 7;
    }
    return 0;
}

int game::isOccupied(int index){ //returns number of people of another color
    int number_of_people = 0;

    if(index != 99){
        for(size_t i = 0; i < player_positions.size(); ++i){
            if(i < color*4 || i >= color*4 + 4){        //Disregard own players
                if(player_positions[i] == index){
                    ++number_of_people;
                }
            }
        }
    }
    return number_of_people;
}

bool game::isGlobe(int index){
    if(index < 52){     //check only the indexes on the board, not in the home streak
        if(index % 13 == 0 || (index - 8) % 13 == 0 || isOccupied(index) > 1){  //if more people of the same team stand on the same spot it counts as globe
            return true;
        }
    }
    return false;
}

void game::send_them_home(int index){
    for(size_t i = 0; i < player_positions.size(); ++i){
        if(i < color*4 || i >= color*4 + 4){        //this way we don't skip one player position
            if(player_positions[i] == index){
                player_positions[i] = -1;
            }
        }
    }
}

void game::move_start(int fixed_piece){
    if(dice_result == 6 && player_positions[fixed_piece] < 0){
        player_positions[fixed_piece] = color*13; //move me to start
        send_them_home(color*13); //send pieces home if they are on our start
    }
}

int game::next_turn(unsigned int delay = 0){
    if(game_complete){
        return 0;
    }
    switch(color){
    case 0:
    case 1:
    case 2:
        ++color;
        break;
    case 3:
    default:
        color = 0;
        break;
    }
    // global_color = color;
    rollDice();
    relative.dice = getDiceRoll();
    relative.pos = relativePosition();
    emit set_color(color);
    emit set_dice_result(dice_result);

    msleep(delay);
    switch(color){
    case 0:
        for(int i = 0 ; i < 4 ; i++){
            if(relative.pos[i] == 99){
                player1_in_goal[i] = 1;
            }
        }
        emit player1_start(relative);
        break;
    case 1:
        for(int i = 0 ; i < 4 ; i++){
            if(relative.pos[i] == 99){
                player2_in_goal[i] = 1;
            }
        }
        emit player2_start(relative);
        break;
    case 2:
        for(int i = 0 ; i < 4 ; i++){
            if(relative.pos[i] == 99){
                player3_in_goal[i] = 1;
            }
        }
        emit player3_start(relative);
        break;
    case 3:
        for(int i = 0 ; i < 4 ; i++){
            if(relative.pos[i] == 99){
                player4_in_goal[i] = 1;
            }
        }
        emit player4_start(relative);
    default:
        break;
    }

    return 0;
}

void game::movePiece(int relative_piece){
    int fixed_piece = rel_to_fixed(relative_piece);     //index of the piece in player_positions
    int modifier = color * 13;
    int relative_pos = player_positions[fixed_piece];
    int target_pos = 0;
    if(player_positions[fixed_piece] == -1){        //if the selected piece is in the safe house, try to move it to start
        move_start(fixed_piece);
    } else {
        //convert to relative position
        if(relative_pos == 99){
        //    std::cout << "I tought this would be it ";
        } else if(relative_pos < modifier) {
            relative_pos = relative_pos + 52 - modifier;
        } else if( relative_pos > 50) {
            relative_pos = relative_pos - color * 5 - 1;
        } else {//if(relative >= modifier)
            relative_pos = relative_pos - modifier;
        }
        if(DEBUG) std::cout << "color: " << color << " pos: " << relative_pos << " + " << dice_result << " = " << relative_pos + dice_result;
        //add dice roll
        relative_pos += dice_result;    //this is relative position of the selected token + the dice number

        int jump = isStar(relative_pos); //return 0 | 6 | 7
        if(jump){
            if(jump + relative_pos == 57){
                relative_pos = 56;
            } else {
                relative_pos += jump;
            }
        }
        //special case checks
        if(relative_pos > 56 && relative_pos < 72){ // go back
            target_pos = 56-(relative_pos-56) + color * 5 + 1; //If the player moves over the goal, it should move backwards
        }else if(relative_pos == 56 || relative_pos >= 99){
            target_pos = 99;
        }else if(relative_pos > 50){ // goal stretch
            target_pos = relative_pos + color * 5 + 1;
        } else {
            int new_pos = relative_pos + color * 13;
            if(new_pos < 52){
                target_pos = new_pos;
            } else { //wrap around
                target_pos = new_pos - 52;  //this is the global position wrap around at the green entry point
            }
        }
        //check for game stuff

        if(isOccupied(target_pos)){
            if(isGlobe(target_pos)){
                target_pos = -1; //send me home
            } else {
                send_them_home(target_pos);
            }
        }
        if(DEBUG) std::cout << " => " << target_pos << std::endl;
        player_positions[fixed_piece] = target_pos;
    }
    std::vector<int> new_relative = relativePosition();
    switch(color){
        case 0:
            emit player1_end(new_relative);
            break;
        case 1:
            emit player2_end(new_relative);
            break;
        case 2:
            emit player3_end(new_relative);
            break;
        case 3:
            emit player4_end(new_relative);
        default:
            break;
    }
    emit update_graphics(player_positions);
}

std::vector<int> game::relativePosition(){
    std::vector<int> relative_positons;
    int modifier = color * 13;

    //from start id to end
    for(int i = color*4; i < player_positions.size(); ++i){
        relative_positons.push_back(player_positions[i]);
    }
    //from 0 to start id
    for(int i = 0; i < color*4; ++i){
        relative_positons.push_back(player_positions[i]);
    }


    for(size_t i = 0; i < relative_positons.size(); ++i){
        if(relative_positons[i] == 99 || relative_positons[i] == -1){
            relative_positons[i] = (relative_positons[i]);
        } else if(relative_positons[i] < modifier) {
            relative_positons[i] = (relative_positons[i]+52-modifier);
        } else if(relative_positons[i] > 50) {
            relative_positons[i] = (relative_positons[i]-color*5-1);
        } else if(relative_positons[i] > modifier) {
            relative_positons[i] = (relative_positons[i]-modifier);
        }
    }
    return std::move(relative_positons);
}

void game::turnComplete(bool win){
    game_complete = false;
    if(win){
            winner[0] = player1_in_goal[0]+player1_in_goal[1]+player1_in_goal[2]+player1_in_goal[3];
            winner[1] = player2_in_goal[0]+player2_in_goal[1]+player2_in_goal[2]+player2_in_goal[3];
            winner[2] = player3_in_goal[0]+player3_in_goal[1]+player3_in_goal[2]+player3_in_goal[3];
            winner[3] = player4_in_goal[0]+player4_in_goal[1]+player4_in_goal[2]+player4_in_goal[3];

            if(color == 0){
                winner[0] = winner[0]+1;
            }
            else if(color == 1){
                winner[1] = winner[1]+1;
            }
            else if(color == 2){
                winner[2] = winner[2]+1;
            }
            else if(color == 3){
                winner[3] = winner[3]+1;
            }

            game_complete = win;

            std::fstream fs;
            fs.open ("result_endstate.txt", std::fstream::in | std::fstream::out | std::fstream::app);

            fs << winner[0] << "," << winner[1] << "," << winner[2] << "," << winner[3] << "\n";
            std::cout << winner[0] << "," << winner[1] << "," << winner[2] << "," << winner[3] << std::endl;
            fs.close();

            std::cout << "WIN" << std::endl;
        }

        turn_complete = true;
        if(game_complete){
            std::cout << "player: " << color << " won" << std::endl;

            emit declare_winner(color);
        }
}

void game::run() {
    if(DEBUG) std::cout << "color:     relative pos => fixed\n";
    while(!game_complete){
        if(turn_complete){
            turn_complete = false;
            msleep(game_delay/4);
            next_turn(game_delay - game_delay/4);
        }
    }
    emit close();
    QThread::exit();
}
