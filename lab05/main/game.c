#include "game.h"
#include "board.h"
#include "graphics.h"
#include "nav.h"
#include "config.h"
#include "com.h"
#include "hw.h"
#include "lcd.h"
#include "joy.h"
#include "pin.h"

// SM States
typedef enum {
    init_st,
    wait_for_role_st,
    wait_for_ack_st,
    new_game_st,
    check_turn_st,
    my_turn_st,
    their_turn_st,
    mark_st,
    wait_restart_st
} game_state_t;

/* packet structs */
typedef enum {
    PACKET_TYPE_CLAIM_X, 
    PACKET_TYPE_ACK_O,  
    PACKET_TYPE_MOVE,    
    PACKET_TYPE_RESTART
} packet_type_t;

typedef struct {
    int8_t r;
    int8_t c;
} move_data_t;

typedef struct {
    packet_type_t type;
    
    union {
        move_data_t move;
    } data;
} game_packet_t;

static game_state_t game_state;
static mark_t current_player;
static mark_t my_player;

static bool user_restart;
static bool opponent_restart;


void game_init(void) {
    game_state = init_st;
    board_clear();
}

void game_tick(void) {
    game_packet_t packet;
    int32_t bytes_read = com_read(&packet, sizeof(packet));

    switch (game_state) {
        int8_t r, c;

        case init_st:
            // Actions
            com_init();
            lcd_fillScreen(CONFIG_BACK_CLR); // Was too long for new_game_st. Moved here and into wait_restart_st.
            graphics_drawGrid(CONFIG_GRID_CLR);
            graphics_drawMessage("Press 'A' to Claim X", CONFIG_MESS_CLR, CONFIG_BACK_CLR);

            // Transitions
            game_state = wait_for_role_st;
            break;

        case wait_for_role_st:
            // Actions

            // Transitions
            if (pin_get_level(HW_BTN_A) == 0) { // User claimed 'X'
                my_player = X_m;
                
                game_packet_t claim_packet;
                claim_packet.type = PACKET_TYPE_CLAIM_X;
                com_write(&claim_packet, sizeof(claim_packet));
                
                graphics_drawMessage("You are X! Waiting for O...", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = wait_for_ack_st;

            } else if (bytes_read == sizeof(packet) && packet.type == PACKET_TYPE_CLAIM_X) { // Opponent claimed 'X'
                my_player = O_m;
                
                game_packet_t ack_packet;
                ack_packet.type = PACKET_TYPE_ACK_O;
                com_write(&ack_packet, sizeof(ack_packet)); //

                graphics_drawMessage("You are O! Waiting for X...", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = new_game_st;
            }
            break;

        case wait_for_ack_st:
            // Actions

            // Transitions
            if (bytes_read == sizeof(packet) && packet.type == PACKET_TYPE_ACK_O) {
                graphics_drawMessage("O has joined! Starting game...", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = new_game_st;
            }
            break;
        
        case new_game_st:
            // Actions
            user_restart = false;
            opponent_restart = false;
            board_clear();
            
            current_player = X_m;

            nav_set_loc(CONFIG_BOARD_R / 2, CONFIG_BOARD_C / 2);
            
            // Transitions
            game_state = check_turn_st;
            break;

        case check_turn_st:
            // Actions

            // Transitions
            if (current_player == my_player) {
                graphics_drawMessage("Your Turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = my_turn_st;
            } else {
                graphics_drawMessage("Waiting for Opponent...", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = their_turn_st;
            }
            break;

        case my_turn_st:
            // Actions

            // Transitions
            if (pin_get_level(HW_BTN_A) == 0) {
                nav_get_loc(&r, &c);

                if (board_set(r, c, current_player)) { // Set mark and transition if valid
                    game_packet_t move_packet;
                    move_packet.type = PACKET_TYPE_MOVE;
                    move_packet.data.move.r = r;
                    move_packet.data.move.c = c;
                    com_write(&move_packet, sizeof(move_packet));
                    
                    game_state = mark_st;
                }
            }
            break;

        case their_turn_st:
            // Actions

            // Transitions
            if (bytes_read == sizeof(packet) && packet.type == PACKET_TYPE_MOVE) {
                r = packet.data.move.r;
                c = packet.data.move.c;

                board_set(r, c, current_player);
                nav_set_loc(r, c);

                game_state = mark_st;
            }
            break;

        case mark_st:
            // Actions
            nav_get_loc(&r, &c);

            if (current_player == X_m) {
                graphics_drawX(r, c, CONFIG_MARK_CLR);
            } else {
                graphics_drawO(r, c, CONFIG_MARK_CLR);
            }

            // Transitions
            if (board_winner(current_player)) {
                if (current_player == X_m) {
                    graphics_drawMessage("X wins! Press start to restart.", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                } else {
                    graphics_drawMessage("O wins! Press start to restart.", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                }
                game_state = wait_restart_st;
            } else if (board_mark_count() == CONFIG_BOARD_SPACES) {
                graphics_drawMessage("It's a draw! Press start to restart.", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                game_state = wait_restart_st;
            } else {
                // Switch players
                current_player = (current_player == X_m) ? O_m : X_m;
                game_state = check_turn_st;
            }
            break;

        case wait_restart_st:
            // Actions
            if (pin_get_level(HW_BTN_START) == 0 && !user_restart) {
                user_restart = true;

                game_packet_t restart_packet;
                restart_packet.type = PACKET_TYPE_RESTART;
                com_write(&restart_packet, sizeof(restart_packet)); //
                
                graphics_drawMessage("Waiting for opponent...", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            }

            if (bytes_read == sizeof(packet) && packet.type == PACKET_TYPE_RESTART) {
                opponent_restart = true;
            }

            // Transitions
            if (user_restart && opponent_restart) {
                lcd_fillScreen(CONFIG_BACK_CLR);
                graphics_drawGrid(CONFIG_GRID_CLR);
                game_state = new_game_st;
            }
            break;

        default:
            game_state = init_st;
            break;
    }
}