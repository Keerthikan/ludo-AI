#include "player_q_learning.h"
using namespace std;
double acc = 0;
bool FileExists( const std::string &Filename )
{
    return access( Filename.c_str(), 0 ) == 0;
}

player_q_learning::player_q_learning()
{
    cout << "q-learning" << endl;
    acc_reward_player1 = 0;
    acc_reward_player2 = 0;
    acc_reward_player3 = 0;
    acc_reward_player4 = 0;
    if(FileExists("trainQ.txt")) // Continue from previous
    {
        int nrows = 7;
        int ncols = 11;
        Q_learning_table = Eigen::MatrixXd(nrows,ncols);
        ifstream fin ("trainQ.txt");
        if (fin.is_open())
        {
            for (int row = 0; row < nrows; row++)
                for (int col = 0; col < ncols; col++)
                {
                    float item = 0.0;
                    fin >> item;
                    Q_learning_table(row, col) = item;
                }
            fin.close();
        }
    }
    else
    {
        Q_learning_table = Eigen::MatrixXd::Zero(7,11);
    }
    update = false;
    cout << "Q_learning_table initiliazed!" << endl;

}

void player_q_learning::calc_possible_actions(float input[9],int current_position, int dice, int token)
{
    cout << "Inside Calc_Input!" << endl;
    int index;
    if(current_position != -1 || current_position >= 99)
    {
        index = current_position + dice;

    }

    // Get out of Home          1 0 0 0 0 0 0 0 0
    if(dice == 6 && current_position == -1)
    {
        input[0] = 1;
        //cout << "Move to Get out of Home" << endl;
    }

    if(current_position != -1){

        // Get in goal              0 1 0 0 0 0 0 0 0
        if(index == 56)
        {
            input[1] = 1;
          //  cout << "move to Goal" << endl;
        }
        // Move to Globe            0 0 0 1 0 0 0 0 0
        // Globes pos: 8 13 21 26 34 39 47
        if(index == 8 || index == 13 || index == 21 || index == 26 || index == 34 || index == 39 || index == 47)
        {
            input[3] = 1;
            //cout << "Move to Globe" << endl;
        }

        // Move to Star             0 0 0 0 1 0 0 0 0
        if(index == 5 || index == 18 || index == 31 || index == 44 || index == 11 || index == 24 || index == 37)
        {
            input[4] = 1;
            //cout << "Move to Star" << endl;
        }

        // Get into safety with an other token   0 0 0 0 0 1 0 0 0
        if(((index == pos_start_of_turn[0])  && (token != 0))|| ((index == pos_start_of_turn[1]) && (token != 1)) || ((index == pos_start_of_turn[2]) && (token != 2)) || ((index == pos_start_of_turn[3]) && (token != 3)) && index != 99)
        {
                input[5] = 1;
              //  cout << "Move to Safety" << endl;
        }

        // Move to Goal via Star    0 1 0 0 1 0 0 0 0
        if(index == 50)
        {
            input[1] = 1;
            input[4] = 1;
            //cout << " Move to Goal via Star" << endl;
        }

        // Get into the Winner Road 0 0 0 0 0 0 1 0 0
        if(index > 50 && index < 56)
        {
            input[6] = 1;
            //cout << "Move to Winner Road" << endl;
        }

        // Kill an opponent
        for(int i = 4 ; i < 16 ; i++ )
            if(index == pos_start_of_turn[i]){
                // Kamikaze if the opponent is on a Globe 0 0 0 0 0 0 0 0 1
                if(index == 8 || index == 13 || index == 21 || index == 26 || index == 34 || index == 39 || index == 47){
                    input[8] = 1;
                    if(input[3] == 1)
                    {
                        cout << "Remove Going to globe!" << endl;
                        input[3] = 0;
                    }
                    //cout << " Move to Globe Kamikaze" << endl;
                    //cout << "player already on: " << i << endl;

                }
                // Kill that Mo-Fo 0 0 0 0 0 0 0 1 0
                else
                {
                    input[7] = 1;
                    if(input[3] == 1)
                    {
                        cout << "Remove Going to globe!" << endl;
                        input[3] = 0;
                    }
                    if(input[4] == 1)
                    {
                        cout << "star" << endl;
                        input[4] = 0;
                    }
                    if(input[6] == 1)
                    {
                        cout << "reset going to winnerRoad to killing someone one winner road" << endl;
                        input[6] = 0;
                    }
                    //cout << "Move to Kill Opponent" << endl;
                }
            }

        // Two opponent on the same square
        //if(input[5] != 1)
        //{
            for(int j = 1 ; j < 4 ; j++)
            {
                // Token 1 and 2
                if(pos_start_of_turn[j*4] == pos_start_of_turn[j*4+1])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square" << j*4 << "  " << j*4+1 << endl;
                    }
                }
                // Token 1 and 3
                else if(pos_start_of_turn[j*4] == pos_start_of_turn[j*4+2])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square" << j*4 << "  " << j*4+2 << endl;
                    }
                }
                // Token 1 and 4
                else if(pos_start_of_turn[j*4] == pos_start_of_turn[j*4+3])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square" << j*4 << "  " << j*4+3 << endl;
                    }
                }
                // Token 2 and 3
                else if(pos_start_of_turn[j*4+1] == pos_start_of_turn[j*4+2])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square" << j*4+1 << "  " << j*4+2 << endl;
                    }
                }
                // Token 2 and 4
                else if(pos_start_of_turn[j*4+1] == pos_start_of_turn[j*4+3])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square: " << j*4+1 << "  " << j*4+3 << endl;
                    }
                }
                // Token 3 and 4
                else if(pos_start_of_turn[j*4+2] == pos_start_of_turn[j*4+3])
                {
                    if(index == pos_start_of_turn[j*4])
                    {
                        input[8] = 1;
                        //cout << "Move to Two opponent on the same square" << j*4+2 << "  " << j*4+3 << endl;
                    }
                }
            }
        //}
        if(input[0] == 0 && input[1] == 0 && input[3] == 0 && input[4] == 0 && input[5] == 0 && input[6] == 0 && input[7] == 0 && input[8] == 0)
        {
            cout << "No Special move found!" << endl;
            if(current_position != 99)
            {
                cout << "Is not in goal" << endl;
                input[2] = 1;
                cout << "Just move" << endl;
            }
            else
            {
                cout << "Already in goal! -  no move possible!" << endl;

            }
        }
    }

}

void player_q_learning::calc_current_state(float input[7], int current_position, int token)
{
    //In Home
    if(current_position == -1)
    {
        input[0] = 1;
        cout << "Token in home" << endl;
    }

    //Globe check
    //Globes pos:  0 8 13 21 26 34 39 47
    if(current_position == 8 || current_position == 13 || current_position == 21 || current_position == 26 || current_position == 34 || current_position == 39 || current_position == 47 || current_position == 0)
    {
        input[1] = 1;
        cout << "Token is on a globe" << endl;
    }

    //Star check
    //Star pos: 5 18 31 44 11 24 37 50
    if(current_position == 5 || current_position == 18 || current_position == 31 || current_position == 44 || current_position == 11 || current_position == 24 || current_position == 37 || current_position == 50 )
    {
        input[2] = 1;
        cout << "Move to Star" << endl;
    }

    //In goal check
    if(current_position == 99 || current_position == 56)
    {
        input[3] = 1;
        cout << "You are in Goal! " << endl;
    }

    //Winner road check
    if(current_position > 50 && current_position < 56)
    {
        input[4] = 1;
        cout << "On Winner Road" << endl;
    }
    // In safety check
    if(current_position != -1 && current_position != 99)
    {
        if(((current_position == pos_start_of_turn[0])  && (token != 0))|| ((current_position == pos_start_of_turn[1]) && (token != 1)) || ((current_position == pos_start_of_turn[2]) && (token != 2)) || ((current_position == pos_start_of_turn[3]) && (token != 3)))
        {
            input[5] = 1;
            cout << "Move to Safety" << endl;
        }
    }
    // On freeSpace check - nothing special
    if(input[0] != 1 && input[1] != 1 && input[2] != 1 && input[3] != 1 && input[4] != 1 && input[5] != 1)
    {
        cout << "Current_position: " << current_position << endl;
        cout << "On free Space: " << endl;
        input[6] = 1;
    }


}

std::vector<int> player_q_learning::current_state_intepreter(float input[7]) // skal lige tjekkes om det nødvendigt med vector of ints
{
    std::vector<int> output;
    if(input[0]  == 1)
    {
        cout << "In Home" << endl;
        output.push_back(0);
    }
    if(input[1]  == 1)
    {
        cout << "On globe" << endl;
        output.push_back(1);
    }
    if(input[2]  == 1)
    {
        cout << "On star" << endl;
        output.push_back(2);
    }
    if(input[3]  == 1)
    {
        cout << "in goal" << endl;
        output.push_back(3);
    }
    if(input[4]  == 1)
    {
        cout << "On winner road" << endl;
        output.push_back(4);
    }
    if(input[5]  == 1)
    {
        cout << "In safety" << endl;
        output.push_back(5);
    }
    if(input[6]  == 1)
    {
        cout << "On FreeSpace" << endl;
        output.push_back(6);
    }
    cout << output.size() << endl;
    return output;
}

std::vector<std::tuple<int, int, int>> player_q_learning::player_state_action_intepreter(float input_states[7], float input_actions[9], int token)
{
    std::vector<std::tuple<int,int,int>> output;
    if(input_states[0]  == 1)
    {
        cout << "Token is in Home" << endl;
        int state = 0;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }

        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }


        cout << "Number of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }

    }
    if(input_states[1]  == 1)
    {
        cout << "Token is on globe" << endl;
        int state = 1;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    if(input_states[2]  == 1)
    {
        cout << "Token is on star" << endl;
        int state = 2;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    if(input_states[3]  == 1)
    {
        cout << "Token is on goal" << endl;
        int state = 3;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    if(input_states[4]  == 1)
    {
        cout << "Token is on winner road" << endl;
        int state = 4;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    if(input_states[5]  == 1)
    {
        cout << "Token is in safety" << endl;
        int state = 5;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    if(input_states[6]  == 1)
    {
        cout << "Token in freespace" << endl;
        int state = 6;
        std::vector<int> actions;
        if(input_actions[0] == 1)
        {
            actions.push_back(0);
            cout << "Move out from home!" << endl;
        }

        if(input_actions[1] == 1)
        {
            actions.push_back(1);
            cout << "Move to goal!" << endl;
        }

        if(input_actions[3] == 1)
        {
            actions.push_back(2);
            cout << "Move to globe" << endl;
        }

        if(input_actions[4] == 1)
        {
            actions.push_back(3);
            cout << "Move to star" << endl;
        }

        if(input_actions[4] == 1 && input_actions[1] == 1 )
        {
            actions.push_back(4);
            cout << "Move to goal via star" << endl;
        }

        if(input_actions[5] == 1)
        {
            actions.push_back(5);
            cout << "Get into safety with other player" << endl;
        }

        if(input_actions[6] == 1)
        {
            actions.push_back(6);
            cout << "Get into winner road" << endl;
        }

        if(input_actions[8] == 1)
        {
            actions.push_back(7);
            cout << "kamikaze if the oppponent is on a globe" << endl;
        }

        if(input_actions[7] == 1)
        {
            actions.push_back(8);
            cout << "Kill that MOFO!" << endl;
        }
        if(input_actions[2] == 1)
        {
           actions.push_back(9);
           cout << "Just move" << endl;
        }
        if(input_actions[0] == 0 &&
                input_actions[1] == 0 &&
                input_actions[2] == 0 &&
                input_actions[3] == 0 &&
                input_actions[4] == 0 &&
                input_actions[5] == 0 &&
                input_actions[6] == 0 &&
                input_actions[7] == 0 &&
                input_actions[8] == 0 )
        {
            cout << "No move possible" << endl;
            actions.push_back(10);
        }
        cout << "Amount of Actions for given state: " << actions.size() << endl;
        for(unsigned int i = 0 ; i < actions.size(); i++)
        {
            int action =actions[i];
            output.push_back(make_tuple(token, state, action));

        }
    }
    return output;
}

void player_q_learning::updateQ(std::tuple<int,int,int,int> player_state_action_i)
{
    double alfa = 0.02; // 0 < alfa <= 1
    double gamma = 0.07; // 0 < gamma <= 1
    int player_played_i = std::get<0>(player_state_action_i);
    int previous_state = std::get<1>(player_state_action_i);
    int performed_action = std::get<2>(player_state_action_i);
    int previous_position = std::get<3>(player_state_action_i);
    int current_position = pos_start_of_turn[player_played_i];
    //int current_position_0 = pos_start_of_turn[0];
    //int current_position_1 = pos_start_of_turn[1];
    //int current_position_2 = pos_start_of_turn[2];
    //int current_position_3 = pos_start_of_turn[3];

    std::vector<int> current;
    float current_state[7];

    calc_current_state(current_state,current_position,player_played_i);
    current = current_state_intepreter(current_state);
    cout << "size of vector: " << current.size() << endl;

    if(current.size() > 1)
    {
        cout << "Multiple states!" << endl;
        //exit(0);
    }

    //int current_player_state = current[0];
    double reward = (current_position * 10)+10; // should be based on everyones position...
    //double reward = (current_position_0 + current_position_1 + current_position_2 + current_position_3) * 10;
    if(previous_position == current_position && previous_state == current[0]  && current_position != -1)
    {
        cout << "Haven't moved!" << endl;
        reward = -1;
    }
    if(current_position == 99 && previous_position != current_position)
    {
        cout << "you are in goal!" << endl;
        reward = 99;
    }

    acc += reward;
    cout <<"Accumulated: "<< acc << endl;
    //std::ofstream reward_debug ("reward.txt" , std::ios_base::app);
    //std::ofstream reward_acc_plot ("reward_plot.csv" , std::ios_base::app);

    //reward_debug << "Reward for #0 " << pos_start_of_turn[0]*10 << " #1: " << pos_start_of_turn[1]*10 <<" #2: "  << pos_start_of_turn[2]*10 <<" #3: " << pos_start_of_turn[3]*10 << " Player Played: "<< player_played_i << endl;
    cout << "Reward for #0 " << pos_start_of_turn[0]*10 << " #1: " << pos_start_of_turn[1]*10 <<" #2: "  << pos_start_of_turn[2]*10 <<" #3: " << pos_start_of_turn[3]*10 << " Player Played: "<< player_played_i << endl;

    //reward_acc_plot << acc << "," << endl;
//    reward_plot_2 << acc_reward_player2 << "," << endl;
//    reward_plot_3 << acc_reward_player3 << "," << endl;
//    reward_plot_4 << acc_reward_player4 << "," << endl;
    cout << "current position: " << current_position << endl;
    cout << "Immediate Reward: " << reward << endl;

    double max = -10000000000000;
    for(unsigned int j = 0; j< current.size() ; j++)
    {
        int current_player_state = current[j];
        for(int i = 0; i< 11 ;  i++)
        {
            //cout << current_player_state << " " << i << endl;
            double test = Q_learning_table(current_player_state,i);
            if(test > max)
            {
                max  = i;
            }
        }
    }
    cout << previous_state  << endl;
    cout << performed_action << endl;
    cout << Q_learning_table.cols() << endl;
    cout << Q_learning_table.rows() << endl;
    cout << "Previous value: " << Q_learning_table(previous_state,performed_action)  << endl;
    Q_learning_table(previous_state,performed_action) += alfa * (reward + gamma * max - Q_learning_table(previous_state,performed_action));
    cout << "Updated value: " << Q_learning_table(previous_state,performed_action)  << endl;
}

int player_q_learning::make_decision()
{
    cout << "NEW EPISODE!" << endl;
    cout << endl;
    cout << endl;
    cout << "-----------------------Update--------------------------" << endl;
    if(update == true)
    {
        cout << "peform update" << endl;
        cout << "previous player played: " << std::get<0>(player_state_action_previous_position) << endl;
        cout << "Previous state: " << std::get<1>(player_state_action_previous_position) << " and action: " << std::get<2>(player_state_action_previous_position) << endl;
        updateQ(player_state_action_previous_position);
        player_state_action.clear();
        std::ofstream log("logfile.txt", std::ios_base::app);
        std::ofstream current_table("trainQ.txt");
        current_table << Q_learning_table << endl;
        log << Q_learning_table << endl;
        log << endl;
        cout << "table saved" << endl;
        update = false;
    }
    player_state_action.clear();
    cout << "-------------------------------------------------------" << endl;
    for(int i = 0 ; i < 4 ; i++)
    {
        if(pos_start_of_turn[i]>=-1 && pos_start_of_turn[i] < 99) // Maybe the if should be changed with no end limit
        {
            cout << "Player token " << i << " with pos " << pos_start_of_turn[i] << endl;

            float input_actions[9] = {0,0,0,0,0,0,0,0,0};

            /*
             * Possible Actions
             * 1 0 0 0 0 0 0 0 0 -  Out from Home                           -Row1
             * 0 1 0 0 0 0 0 0 0 -  Get in goal                             -Row2
             * 0 0 0 1 0 0 0 0 0 -  Move to Globe                           -Row3
             * 0 0 0 0 1 0 0 0 0 -  Move to Star                            -Row4
             * 0 1 0 0 1 0 0 0 0 -  Move to Goal via Star                   -Row5
             * 0 0 0 0 0 1 0 0 0 -  Get into safety with an other token     -Row6
             * 0 0 0 0 0 0 1 0 0 -  Get into the Winner Road                -Row7
             * 0 0 0 0 0 0 0 0 1 -  Kamikaze if the opponent is on a Globe  -Row8
             * 0 0 0 0 0 0 0 1 0 -  Kill that Mo-Fo                         -Row9
             * 0 0 1 0 0 0 0 0 0 -  Just move                               -Row10
             * 0 0 0 0 0 0 0 0 0 -  No move possible                        -Row11
             */

            float input_states[7] = {0,0,0,0,0,0,0};

            /*
             * Possible States
             * 1 0 0 0 0 0 0  -    In home
             * 0 1 0 0 0 0 0  -    On globe
             * 0 0 1 0 0 0 0  -    On A star
             * 0 0 0 1 0 0 0  -    In Goal
             * 0 0 0 0 1 0 0  -    On the Winner Road
             * 0 0 0 0 0 1 0  -    In safety with same colored player
             * 0 0 0 0 0 0 1  -    On Freespace
             */

            //cout <<"Making decision:  "<< pos_start_of_turn[i] << endl;
            calc_possible_actions(input_actions,pos_start_of_turn[i],dice_roll, i);
            calc_current_state(input_states,pos_start_of_turn[i],i);

            cout << "Token #" << i << endl;
            cout << "Start position: " << pos_start_of_turn[i] << endl;
            cout << "dice_roll: "<< dice_roll << endl;

            cout <<"Actions possible: "<<  input_actions[0] <<
                                input_actions[1] << input_actions[2] <<
                                input_actions[3] << input_actions[4] <<
                                input_actions[5] << input_actions[6] <<
                                input_actions[7] << input_actions[8] << endl;

            cout << "Current state: " << input_states[0] << input_states[1]
                                      << input_states[2] << input_states[2]
                                      << input_states[3] << input_states[4]
                                      << input_states[5] << input_states[6] << endl;
            std::vector<std::tuple<int,int,int>> token_i = player_state_action_intepreter(input_states,input_actions,i);
            player_state_action.insert(player_state_action.end(), token_i.begin(),token_i.end());
        }
        cout << endl;
    }

    update = false;
    cout << "size after all tokens!: "<<player_state_action.size() << endl;

    player_state_action_previous_position = e_greedy(0); // 0 = greedy , 1 = random

    cout << "Player: " << player_played << " In state: " << std::get<1>(player_state_action_previous_position) << " Peforms action: " << std::get<2>(player_state_action_previous_position) << endl;
    if(pos_start_of_turn[player_played]+dice_roll == 56)
    {
        cout << "Player is in goal" << endl;
        cout << "In goal" << endl;
    }
    return player_played;
}

std::tuple<int,int,int,int > player_q_learning::e_greedy(double epsilon)
{
    double limit  = epsilon*100;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1 , 100);
    if(dist(mt)< limit)
    {
        cout << "Choose Random!" << endl;
        //return random
        std::random_device player_r;
        std::mt19937 player_mt(player_r());
        std::uniform_int_distribution<int> dist_p(0 , player_state_action.size()-1);
        //
        std::random_device action_r;
        std::mt19937 action_mt(action_r());
        std::uniform_int_distribution<int> dist_a(0 , 10);
        //
        int player = dist_p(player_mt);
        int state = std::get<1>(player_state_action[player]);
        int action = dist_a(action_mt);
        player_played = player;
        int previous_position = pos_start_of_turn[player_played];
        return make_tuple(player,state,action,previous_position);
    }
    else
    {
        //Return max
        double max = -1000000;
        int position;
        for(unsigned int i = 0 ; i < player_state_action.size(); i++)
        {
            int player = std::get<0>(player_state_action[i]);
            int state = std::get<1>(player_state_action[i]);
            int action = std::get<2>(player_state_action[i]);
            double test = Q_learning_table(state,action);
            cout << test;
            if(test > max)
            {
                max = test;
                position = i;
                player_played = player;
            }
                cout << endl;
        }
        int previous_position = pos_start_of_turn[player_played];
        int state = std::get<1>(player_state_action[position]);
        int action = std::get<2>(player_state_action[position]);
        return make_tuple(player_played,state,action,previous_position);
    }

}

void player_q_learning::start_turn(positions_and_dice relative){
    pos_start_of_turn = relative.pos;
    //cout << "Inside a For loop" << endl;
    dice_roll = relative.dice;
    int decision = make_decision();
    emit select_piece(decision);
}

void player_q_learning::post_game_analysis(std::vector<int> relative_pos){
    pos_end_of_turn = relative_pos;
    bool game_complete = true;
    for(int i = 0; i < 4; ++i){
        if(pos_end_of_turn[i] < 99){
            game_complete = false;
        }
    }
    emit turn_complete(game_complete);
}
