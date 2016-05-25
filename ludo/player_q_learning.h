#ifndef PLAYER_Q_LEARNING_H
#define PLAYER_Q_LEARNING_H
#include <QObject>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <fstream>
#include "positions_and_dice.h"
#include <random>

class player_q_learning : public QObject {
    Q_OBJECT
private:
    Eigen::MatrixXd Q_learning_table;
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_end_of_turn;
    int dice_roll;
    int make_decision();
    int current_state_intepreter(float input[7]);
    std::vector<std::tuple<int,int,int>> player_state_action;
    std::tuple<int,int,int,int> player_state_action_previous_position;
    void updateQ(std::tuple<int, int, int, int> player_state_action_i);
    int player_played;
    void calc_possible_actions(float input[9], int current_position, int dice, int token);
    void calc_current_state(float input[7], int current_position, int token);
    std::vector<std::tuple<int, int, int>> player_state_action_intepreter(float input_states[7], float input_actions[9], int token);
    bool update;
    std::tuple<int, int, int, int> e_greedy(double epsilon);
public:
    player_q_learning();
    double acc_reward_player1;
    int acc_reward_player2;
    int acc_reward_player3;
    int acc_reward_player4;
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

#endif // PLAYER_Q_LEARNING_H


