#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "defs.h"
#include "graphics.h"

SDL_Event event;

Graphics graphics;


class game_timer: std::chrono::high_resolution_clock {
    const time_point start_time;
public:
    game_timer(): start_time(now()) {}
    rep elapsed_time() const { return std::chrono::duration_cast<std::chrono::milliseconds>(now()-start_time).count();
    }
};

int total_time = 0, last_time = 0;
struct texture_array{
    SDL_Texture* texture;
};
std::vector <texture_array> platform_type, deco_type, spike_type;

SDL_Color White = {255,255,255};
SDL_Color Purple = {255,0,255};
std::string deaths = "Deaths: 0";

TTF_Font* title_font = nullptr;
TTF_Font* options_font = nullptr;

SDL_Texture* play_game_button = nullptr;
SDL_Texture* quit_game_button = nullptr;
SDL_Texture* how_to_play_button = nullptr;
SDL_Texture* main_menu_button = nullptr;
SDL_Texture* resume_button = nullptr;
SDL_Texture* leaderboards_button = nullptr;
//vector <SDL_Texture*>


SDL_Surface* play_game = nullptr;
SDL_Surface* quit_game = nullptr;
SDL_Surface* main_menu = nullptr;
SDL_Surface* resume = nullptr;
SDL_Surface* leaderboards = nullptr;
SDL_Surface* how_to_play = nullptr;

SDL_Texture* game_name_text = nullptr;
SDL_Texture* death_text = nullptr;
SDL_Texture* timer_text = nullptr;
SDL_Texture* instructions_text = nullptr;
SDL_Texture* winning_text = nullptr;
SDL_Texture* leaderboards_text = nullptr;
SDL_Texture* pause_text = nullptr;
SDL_Texture* enter_name_text = nullptr;
SDL_Texture* name_text = nullptr;
SDL_Texture* time_taken_text = nullptr;
SDL_Texture* no_deaths_text = nullptr;

SDL_Surface* game_name = nullptr;
SDL_Surface* death_count = nullptr;
SDL_Surface* timer = nullptr;
SDL_Surface* instructions = nullptr;
SDL_Surface* winning = nullptr;
SDL_Surface* pause = nullptr;
SDL_Surface* enter_name = nullptr;
SDL_Surface* name = nullptr;
SDL_Surface* time_taken = nullptr;
SDL_Surface* no_deaths = nullptr;

SDL_Texture* background = nullptr;
SDL_Texture* player_sprite = nullptr;
SDL_Texture* platform_sprite = nullptr;
SDL_Texture* red_platform_sprite = nullptr;
SDL_Texture* blue_platform_sprite = nullptr;
SDL_Texture* spike_sprite = nullptr;
SDL_Texture* goal_sprite = nullptr;
SDL_Texture* red_platform_sprite_disabled = nullptr;
SDL_Texture* blue_platform_sprite_disabled = nullptr;
SDL_Texture* h2p = nullptr;

SDL_Surface* text_surface = nullptr;
SDL_Texture* text_texture = nullptr;

int mouse_x, mouse_y, game_choice = 0;
const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
int death_counter = 0, last_death_count;

struct Player{
    int posx, posy, vx = 0, vy = 0, acc = 0;
        void create(int x, int y){
            posx = x; posy = y;
        }
        bool jump_flag = false, space_flag = false;

        void move_horizontal(horizontal st){
            acc = std::min(acc+ACCEL, SPEED);
            if (st == 0) acc = std::max(acc-2, 0);
            vx = st*acc;
        }
        void jump(){
            vy = INITIAL_JUMP_HEIGHT;
        }

        void stop_moving(){
            vx = 0;
        }
        void move(){
            posx += vx;
            if (jump_flag) vy = 0;
            posy += vy;
            vy += FALL_ACCEL;
        }
};

struct Goal{
    int posx, posy;
    void create(int x, int y){
        posx = x; posy = y;
    }
};

struct Platform{
    int posx, posy, type; bool state;
    void create(int x, int y, int plat_type){
        posx = x; posy = y; type = plat_type;
        state = (plat_type != -1);
    }
};

struct Spike{
    int posx, posy, type; bool state;
    void create(int x, int y, int plat_type){
        posx = x; posy = y; type = plat_type;
        state = (plat_type != -1);
    }
};

/*struct Coin{
    int posx, posy;
    void create(int x, int y){
        posx = x; posy = y;
    }
};*/

struct Level{
    Player player, init_player; Goal goal; std::vector <Platform> platform; std::vector <Spike> spike; /*std::vector <Coin> coin*/;
};

Level level[10]; int current_level = 0;

struct best_scores{
    std::string name;
    int time, deaths;

    SDL_Texture* name_texture = nullptr;
    SDL_Texture* time_texture = nullptr;
    SDL_Texture* deaths_texture = nullptr;
};

bool cmp(best_scores p1, best_scores p2){
    if (p1.time != p2.time) return (p1.time < p2.time);
    if (p1.deaths != p2.deaths) return (p1.deaths < p2.deaths);
    return (p1.name < p2.name);
}


bool in_range(const Player player1, const Platform platform1, const int direction){
    if (direction) return (player1.posy < platform1.posy+PLATFORM_SIZE and player1.posy+CUBE_SIZE > platform1.posy);
    return (player1.posx < platform1.posx+PLATFORM_SIZE and player1.posx+CUBE_SIZE > platform1.posx);
}

void check_collision_ground(Player &player1){
    //Checking the collision between the player and the ground
    if (player1.vx > 0 and player1.posx+CUBE_SIZE+player1.vx > SCREEN_WIDTH){
        player1.vx = 0;
        player1.posx = SCREEN_WIDTH-CUBE_SIZE;
    }
    if (player1.vx < 0 and player1.posx+player1.vx < 0){
        player1.vx = 0;
        player1.posx = 0;
    }
    if (player1.vy > 0 and player1.posy+CUBE_SIZE+player1.vy > SCREEN_HEIGHT){
        player1.vy = 0;
        player1.posy = SCREEN_HEIGHT-CUBE_SIZE;
        player1.jump_flag = true;
    }
    if (player1.vy < 0 and player1.posy+player1.vy < 0){
        player1.vy = 0;
        player1.posy = 0;
    }
}

void check_collision_platform(Player &player1, const Platform plat1){
    if (!plat1.state) return;
    //Checking the collision between the player and the platform in the X axis
    if (player1.vx > 0 and player1.posx+CUBE_SIZE+player1.vx > plat1.posx and player1.posx-CUBE_SIZE+player1.vx < plat1.posx and in_range(player1,plat1,1)){
        player1.vx = 0;
        player1.posx = plat1.posx-PLATFORM_SIZE;
    }
    if (player1.vx < 0 and player1.posx-CUBE_SIZE+player1.vx < plat1.posx and player1.posx+CUBE_SIZE+player1.vx > plat1.posx and in_range(player1,plat1,1)){
        player1.vx = 0;
        player1.posx = plat1.posx+PLATFORM_SIZE;
    }
    //Checking the collision between the player and the platform in the Y axis.
    if (player1.vy > 0 and player1.posy+CUBE_SIZE+player1.vy > plat1.posy and player1.posy-CUBE_SIZE+player1.vy < plat1.posy){
        if (in_range(player1,plat1,0)){
            player1.vy = 0;
            player1.posy = plat1.posy-PLATFORM_SIZE;
            player1.jump_flag = true;
        }
        else player1.jump_flag = false;
    }
    if (player1.vy < 0 and player1.posy-CUBE_SIZE+player1.vy < plat1.posy and player1.posy+CUBE_SIZE+player1.vy > plat1.posy and in_range(player1,plat1,0)){
        player1.vy = 0;
        player1.posy = plat1.posy+PLATFORM_SIZE;
    }
    return;
}

/*bool check_collsiion_coin(const Player player1, const coin coin1){
    return ((player1.posy < coin1.posy+PLATFORM_SIZE and player1.posy+CUBE_SIZE > coin1.posy) and (player1.posx < coin1.posx+PLATFORM_SIZE and player1.posx+CUBE_SIZE > coin1.posx));
}*/

bool check_collision_spike(const Player player1, const Spike spike1){
    if (!spike1.state) return false;
    //Checking the collision between the player and spikes
    if (player1.posx+player1.vx > spike1.posx+SPIKE_RIGHT_HITBOX or spike1.posx+SPIKE_LEFT_HITBOX > player1.posx+player1.vx+CUBE_SIZE) return false;
    if (player1.posy+player1.vy > spike1.posy+SPIKE_BOTTOM_HITBOX or spike1.posy+SPIKE_TOP_HITBOX > player1.posy+player1.vy+CUBE_SIZE) return false;
    return true;
}

bool check_collision_goal(const Player player1, const Goal goal1){
    //Checking the collision between the player and the goal
    return ((player1.posy < goal1.posy+PLATFORM_SIZE and player1.posy+CUBE_SIZE > goal1.posy) and (player1.posx < goal1.posx+PLATFORM_SIZE and player1.posx+CUBE_SIZE > goal1.posx));
}

bool check_switch_state(Player &player1){
    //static int cnt = 1;
    //Check the state of the red/blue colored platforms and spikes
    if (currentKeyStates[SDL_SCANCODE_SPACE] and !player1.space_flag){
        player1.space_flag = true;
        return true;
    }
    if (!currentKeyStates[SDL_SCANCODE_SPACE]) player1.space_flag = false;
    return false;
}

void check_keypress(Player &player1){
    if (!(currentKeyStates[SDL_SCANCODE_LEFT] ^ currentKeyStates[SDL_SCANCODE_RIGHT])) player1.move_horizontal(NO_H_MOVE);
    if (currentKeyStates[SDL_SCANCODE_LEFT] and player1.posx >= 0) player1.move_horizontal(LEFT_MOVE);
    if (currentKeyStates[SDL_SCANCODE_RIGHT] and player1.posx <= SCREEN_WIDTH-40) player1.move_horizontal(RIGHT_MOVE);
    if ((currentKeyStates[SDL_SCANCODE_UP]) and player1.jump_flag) {
        player1.jump();
        player1.jump_flag = false;
    }
}

bool escape_hold = false;

bool check_escape_key(){
    if (currentKeyStates[SDL_SCANCODE_ESCAPE] and !escape_hold){
        escape_hold = true;
        return true;
    }
    if (!currentKeyStates[SDL_SCANCODE_ESCAPE]){
        escape_hold = false;
    }
    return false;

}

int game_loop(Level &CLevel){
    check_keypress(CLevel.player);
    if (check_collision_goal(CLevel.player, CLevel.goal)){
        if (current_level == LEVEL_COUNT - 1) return 0;
        else{
            return 2;
        }
    }
    check_collision_ground(CLevel.player);
    if (check_switch_state(CLevel.player)){
        for (auto &it: CLevel.platform){
            if (it.type == 0 or it.type == -1){
                it.state = (!it.state);
            }
        }
    }
    for (auto &it: CLevel.spike){
        //Todo: add a death counter
        if (check_collision_spike(CLevel.player, it)){
            CLevel.player = CLevel.init_player;
            death_counter++;
            break;
        }
    }
    for (auto it: CLevel.platform){
        check_collision_platform(CLevel.player, it);
    }

    CLevel.player.move();

    return 1;
}

int main_menu_loop(){
    while(true){
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);
            if (!escape_hold and (check_escape_key())){
                return -1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
                if (mouse_x >= 250 and mouse_x <= 310 and mouse_y >= 260 and mouse_y <= 288){
                    return 1;
                }
                if (mouse_x >= 250 and mouse_x <= 400 and mouse_y >= 300 and mouse_y <= 318){
                    SDL_RenderClear(graphics.renderer);
                    return 2;
                }
                if (mouse_x >= 250 and mouse_x <= 350 and mouse_y >= 340 and mouse_y <= 368){
                    return 3;
                }
                if (mouse_x >= 250 and mouse_x <= 310 and mouse_y >= 380 and mouse_y <= 408){
                    return -1;
                }
            }
        }
        graphics.renderTexture(game_name_text, 200, 180);
        graphics.renderTexture(play_game_button, 250, 260);
        graphics.renderTexture(how_to_play_button, 250, 300);
        graphics.renderTexture(leaderboards_button, 250, 340);
        graphics.renderTexture(quit_game_button, 250, 380);
        graphics.presentScene();
        SDL_Delay(10);
    }

}

int instructions_loop(){
    while (true){
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);
            if (currentKeyStates[SDL_SCANCODE_ESCAPE]){
                return 0;
            }
            //std::cerr << mouse_x << " " << mouse_y << std::endl;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                if (mouse_x >= 100 and mouse_x <= 220 and mouse_y >= 520 and mouse_y <= 548){
                    SDL_RenderClear(graphics.renderer);
                    return 0;
                }
                if (mouse_x >= 540 and mouse_x <= 660 and mouse_y >= 520 and mouse_y <= 548){
                    SDL_RenderClear(graphics.renderer);
                    return 1;
                }
            }
        }
        graphics.renderTexture(h2p,0,0);
        graphics.renderTexture(main_menu_button, 100, 520);
        graphics.renderTexture(play_game_button, 540, 520);
        graphics.presentScene();
        SDL_Delay(10);
    }
    return 0;
}

int pause_loop(){
    while(true){
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);
            //std::cout << escape_hold << " " << check_escape_key() << std::endl;
            if (check_escape_key()){
                return 1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
                if (mouse_x >= 250 and mouse_x <= 310 and mouse_y >= 260 and mouse_y <= 288){
                    return 1;
                }
                if (mouse_x >= 250 and mouse_x <= 400 and mouse_y >= 300 and mouse_y <= 318){
                    SDL_RenderClear(graphics.renderer);
                    return 0;
                }
                if (mouse_x >= 250 and mouse_x <= 310 and mouse_y >= 340 and mouse_y <= 368){
                    return -1;
                }
            }
        }
        graphics.renderTexture(pause_text, 280, 180);
        graphics.renderTexture(resume_button, 250, 260);
        graphics.renderTexture(main_menu_button, 250, 300);
        graphics.renderTexture(quit_game_button, 250, 340);
        graphics.presentScene();
        SDL_Delay(10);
    }
}

int win_loop(){
    std::string text = "A";
    text_surface = TTF_RenderText_Solid(options_font, text.c_str(), White);
    text_texture = SDL_CreateTextureFromSurface(graphics.renderer, text_surface);
    SDL_StartTextInput();
    while(true){
        SDL_RenderClear(graphics.renderer);
        bool renderText = false;
        while (SDL_PollEvent(&event)) {
					if(event.type==SDL_KEYDOWN)
					{
						if( event.key.keysym.sym == SDLK_BACKSPACE && text.length() > 0 )
						{
							text.pop_back();
							renderText = true;
						}
						else if( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
						{
							SDL_SetClipboardText( text.c_str() );
						}
						else if( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
						{
							char* tempText = SDL_GetClipboardText();
							text = tempText;
							SDL_free( tempText );

							renderText = true;
						}
					}
					else if( event.type == SDL_TEXTINPUT )
					{
						if( !( SDL_GetModState() & KMOD_CTRL && ( event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' || event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) ) )
						{
							text += event.text.text;
							renderText = true;
						}
					}

            SDL_GetMouseState(&mouse_x, &mouse_y);

            if (!escape_hold and (check_escape_key())){
                return 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
                if (mouse_x >= 100 and mouse_x <= 250 and mouse_y >= 400 and mouse_y <= 428){
                    SDL_RenderClear(graphics.renderer);
                    std::vector <best_scores> top_scores;
                    best_scores current_player;
                    current_player.name = text;
                    current_player.time = last_time;
                    current_player.deaths = last_death_count;
                    top_scores.push_back(current_player);
                    const std::string file_name = "leaderboards.txt";
                    std::ifstream file(file_name);
                    best_scores bs;
                    if (file.is_open()){
                        while (file >> bs.name >> bs.time >> bs.deaths){
                            top_scores.push_back(bs);
                        }
                    }
                    else{
                            std::cout << "CRITICAL ERROR" << std::endl;
                            return -1;
                    }
                    sort(top_scores.begin(), top_scores.end(), cmp);
                    file.close();
                    if (top_scores.size() >= 6) top_scores.pop_back();
                    std::ofstream file1(file_name);
                    for (auto it: top_scores){
                        file1 << it.name << " " << it.time << " " << it.deaths << '\n';
                    }
                    file1.close();
                    return 0;
                }
                if (mouse_x >= 400 and mouse_x <= 520 and mouse_y >= 400 and mouse_y <= 428){
                    SDL_RenderClear(graphics.renderer);
                    std::vector <best_scores> top_scores;
                    best_scores current_player;
                    current_player.name = text;
                    current_player.time = last_time;
                    current_player.deaths = last_death_count;
                    top_scores.push_back(current_player);
                    const std::string file_name = "leaderboards.txt";
                    std::ifstream file(file_name);
                    best_scores bs;
                    if (file.is_open()){
                        while (file >> bs.name >> bs.time >> bs.deaths){
                            top_scores.push_back(bs);
                        }
                    }
                    else{
                            std::cout << "CRITICAL ERROR" << std::endl;
                            return -1;
                    }
                    sort(top_scores.begin(), top_scores.end(), cmp);
                    file.close();
                    if (top_scores.size() >= 6) top_scores.pop_back();
                    std::ofstream file1(file_name);
                    for (auto it: top_scores){
                        file1 << it.name << " " << it.time << " " << it.deaths << '\n';
                    }
                    file1.close();
                    return -1;
                }
            }
        }
        if( renderText){
                if( text != "" )
                {
                    text_surface = TTF_RenderText_Solid(options_font, text.c_str(), White);
                    text_texture = SDL_CreateTextureFromSurface(graphics.renderer, text_surface);
                }
                else
                {
                    text_surface = TTF_RenderText_Solid(options_font, "", White);
                    text_texture = SDL_CreateTextureFromSurface(graphics.renderer, text_surface);
                }
            }

        graphics.renderTexture(text_texture, 200, 300);
        graphics.renderTexture(winning_text, 0, 180);
        graphics.renderTexture(enter_name_text, 200, 240);
        graphics.renderTexture(main_menu_button, 150, 400);
        graphics.renderTexture(quit_game_button, 400, 400);
        graphics.presentScene();
        SDL_Delay(10);
    }
}

int leaderboard_loop(){
    const std::string file_name = "leaderboards.txt";
    std::vector <best_scores> top_scores;
    std::ifstream file(file_name);
    best_scores bs;
    if (file.is_open()){
        while (file >> bs.name >> bs.time >> bs.deaths){
            top_scores.push_back(bs);
        }
    }
    else{
            std::cout << "CRITICAL ERROR" << std::endl;
            return 1;
    }
    file.close();

    for (auto &it: top_scores){
            int minutes = it.time/60000;
            int seconds = it.time/1000 - minutes*60;
            int miliseconds = it.time%1000;
            std::string full_time = "";
            if (minutes < 10) full_time += '0';
            full_time += std::to_string(minutes).c_str();
            full_time += ':';
            if (seconds < 10) full_time += '0';
            full_time += std::to_string(seconds).c_str();
            full_time += ':';
            full_time += std::to_string(miliseconds).c_str();
            SDL_Surface* name1 = TTF_RenderText_Solid(options_font, (it.name).c_str(), White);
            SDL_Surface* time1 = TTF_RenderText_Solid(options_font, (full_time).c_str(), White);
            SDL_Surface* deaths1 = TTF_RenderText_Solid(options_font, std::to_string(it.deaths).c_str(), White);
            it.name_texture = SDL_CreateTextureFromSurface(graphics.renderer, name1);
            it.time_texture = SDL_CreateTextureFromSurface(graphics.renderer, time1);
            it.deaths_texture = SDL_CreateTextureFromSurface(graphics.renderer, deaths1);
            //std::cout << (it.name_texture == nullptr) << std::endl;
    }
    //SDL_Surface* text_box = TTF_RenderText_Solid(options_font, "")
    //std::cout << (top_scores[1].name_texture == nullptr) << std::endl;

    while(true){
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);
            //std::cout << escape_hold << " " << check_escape_key() << std::endl;
            if (check_escape_key()){
                return 0;
            }


            if (event.type == SDL_MOUSEBUTTONDOWN){
                if (mouse_x >= 100 and mouse_x <= 250 and mouse_y >= 520 and mouse_y <= 548){
                    SDL_RenderClear(graphics.renderer);
                    return 0;
                }
                if (mouse_x >= 540 and mouse_x <= 690 and mouse_y >= 520 and mouse_y <= 548){
                    return -1;
                }
            }
        }
        graphics.renderTexture(name_text, 120, 220);
        graphics.renderTexture(time_taken_text, 340, 220);
        graphics.renderTexture(no_deaths_text, 560, 220);
        int i = 270;
        for (auto it: top_scores){
            //std::cout << (it.name_texture == nullptr) << std::endl;
            graphics.renderTexture(it.name_texture, 120, i);
            graphics.renderTexture(it.time_texture, 340, i);
            graphics.renderTexture(it.deaths_texture, 560, i);
            i += 50;
        }

        graphics.renderTexture(leaderboards_text, 350, 180);
        graphics.renderTexture(main_menu_button, 100, 520);
        graphics.renderTexture(quit_game_button, 540, 520);
        graphics.presentScene();
        SDL_Delay(10);
    }

}

int main_game_loop(){
    game_timer t;
    death_counter = 0;
    SDL_SetTextureBlendMode(player_sprite, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(graphics.renderer);
    Level Clevel = level[0];
    int prev_death_counter = 0, prev_player_posx = Clevel.player.posx, prev_player_posy = Clevel.player.posy;
    while (true){
        graphics.prepareScene(NULL);
        while (SDL_PollEvent(&event)) {
            if (check_escape_key()){
                int pause_option = pause_loop();
                if (pause_option == 0) return 0;
                if (pause_option == -1) return -1;
                if (pause_option == 1) continue;
            }
        }
        prev_death_counter = death_counter;
        prev_player_posx = Clevel.player.posx, prev_player_posy = Clevel.player.posy;
        int current_game_state = game_loop(Clevel);
        if (current_game_state == 0){
            last_time = total_time;
            last_death_count = death_counter;
            current_level = 0;
            return win_loop();
        }
        else if (current_game_state == 2){
            current_level++;
            if (current_level == LEVEL_COUNT){
                return win_loop();
            }
            Clevel = level[current_level];
            current_game_state = 1;
            continue;
        }
        if (prev_death_counter != death_counter){
            int opacity = 255;
            while (opacity -= 5){
                graphics.renderTexture(background,0,0);

                total_time = t.elapsed_time();
                int minutes = (total_time)/60000;
                int seconds = (total_time)/1000 - minutes*60;
                int miliseconds = (total_time)%1000;
                std::string full_time = "Time: ";
                if (minutes < 10) full_time += '0';
                full_time += std::to_string(minutes).c_str();
                full_time += ':';
                if (seconds < 10) full_time += '0';
                full_time += std::to_string(seconds).c_str();
                full_time += ':';
                full_time += std::to_string(miliseconds).c_str();

                SDL_Surface* timer_count = TTF_RenderText_Solid (options_font, full_time.c_str(), White);
                SDL_Texture* timer_text = SDL_CreateTextureFromSurface(graphics.renderer, timer_count);
                graphics.renderTexture(timer_text,0,0);

                graphics.renderTexture(death_text, 660, 0);
                graphics.renderTexture(goal_sprite, Clevel.goal.posx, Clevel.goal.posy);

                //SDL_SetTextureAlphaMod(player_sprite, opacity);
                for (auto it: Clevel.platform){
                    if (it.state){
                        if (it.type >= 1){
                            graphics.renderTexture(platform_type[it.type-1].texture, it.posx, it.posy);
                        }
                        else if (it.type == 0){
                            graphics.renderTexture(red_platform_sprite, it.posx, it.posy);
                        }
                        else if (it.type == -1){
                            graphics.renderTexture(blue_platform_sprite, it.posx, it.posy);
                        }
                    }
                    else{
                        if (it.type == 0){
                            graphics.renderTexture(red_platform_sprite_disabled, it.posx, it.posy);
                        }
                        else if (it.type == -1){
                            graphics.renderTexture(blue_platform_sprite_disabled, it.posx, it.posy);
                        }
                    }
                }
                for (auto it: Clevel.spike){
                    graphics.renderTexture(spike_sprite, it.posx, it.posy);
                }
                SDL_SetTextureAlphaMod(player_sprite, opacity);
                graphics.renderTexture(player_sprite, prev_player_posx, prev_player_posy);
                SDL_RenderPresent(graphics.renderer);
                SDL_Delay(10);
            }
        }
        total_time = t.elapsed_time();
        int minutes = (total_time)/60000;
        int seconds = (total_time)/1000 - minutes*60;
        int miliseconds = (total_time)%1000;
        std::string full_time = "Time: ";
        if (minutes < 10) full_time += '0';
        full_time += std::to_string(minutes).c_str();
        full_time += ':';
        if (seconds < 10) full_time += '0';
        full_time += std::to_string(seconds).c_str();
        full_time += ':';
        full_time += std::to_string(miliseconds).c_str();

        SDL_Surface* timer_count = TTF_RenderText_Solid (options_font, full_time.c_str(), White);
        SDL_Texture* timer_text = SDL_CreateTextureFromSurface(graphics.renderer, timer_count);
        graphics.renderTexture(timer_text, 0, 0);

        deaths = "Deaths: " + std::to_string(death_counter);
        death_count = TTF_RenderText_Solid(options_font, deaths.c_str(), White);
        death_text = SDL_CreateTextureFromSurface(graphics.renderer, death_count);
        graphics.renderTexture(death_text, 660, 0);
        graphics.renderTexture(goal_sprite, Clevel.goal.posx, Clevel.goal.posy);
            for (auto it: Clevel.platform){
                if (it.state){
                    if (it.type >= 1){
                        //std::cout << (platform_type[it.type-1].texture == nullptr) << std::endl;
                        graphics.renderTexture(platform_type[it.type-1].texture, it.posx, it.posy);
                    }
                    else if (it.type == 0){
                        graphics.renderTexture(red_platform_sprite, it.posx, it.posy);
                    }
                    else if (it.type == -1){
                        graphics.renderTexture(blue_platform_sprite, it.posx, it.posy);
                    }
                }
                else{
                    if (it.type == 0){
                        graphics.renderTexture(red_platform_sprite_disabled, it.posx, it.posy);
                    }
                    else if (it.type == -1){
                        graphics.renderTexture(blue_platform_sprite_disabled, it.posx, it.posy);
                    }
                }
            }
        for (auto it: Clevel.spike){
            graphics.renderTexture(spike_sprite, it.posx, it.posy);
        }

        SDL_SetTextureAlphaMod(player_sprite, 255);
        graphics.renderTexture(player_sprite, Clevel.player.posx, Clevel.player.posy);
        graphics.presentScene();
        SDL_Delay(10);
    }
}

void texture_initialization(){
    graphics.init();
        //Text textures
        title_font = TTF_OpenFont("Qdbettercomicsans-jEEeG.ttf", 44);
        options_font = TTF_OpenFont("Qdbettercomicsans-jEEeG.ttf", 28);

        play_game = TTF_RenderText_Solid(options_font, "Play", White);
        play_game_button = SDL_CreateTextureFromSurface(graphics.renderer, play_game);
        how_to_play = TTF_RenderText_Solid(options_font, "How to Play", White);
        how_to_play_button = SDL_CreateTextureFromSurface(graphics.renderer, how_to_play);
        quit_game = TTF_RenderText_Solid(options_font, "Quit", White);
        quit_game_button = SDL_CreateTextureFromSurface(graphics.renderer, quit_game);
        main_menu = TTF_RenderText_Solid(options_font, "Main Menu", White);
        main_menu_button = SDL_CreateTextureFromSurface(graphics.renderer, main_menu);
        resume = TTF_RenderText_Solid(options_font, "Resume", White);
        resume_button = SDL_CreateTextureFromSurface(graphics.renderer, resume);


        game_name = TTF_RenderText_Solid(title_font, "World's Hardest Platformer", Purple);
        game_name_text = SDL_CreateTextureFromSurface(graphics.renderer, game_name);
        death_count = TTF_RenderText_Solid(options_font, deaths.c_str(), White);
        death_text = SDL_CreateTextureFromSurface(graphics.renderer, death_count);
        instructions = TTF_RenderText_Solid(options_font, "testtext", White);
        instructions_text = SDL_CreateTextureFromSurface(graphics.renderer, instructions);
        winning = TTF_RenderText_Solid(title_font, "Congratulations! You've finished the game!", White);
        winning_text = SDL_CreateTextureFromSurface(graphics.renderer, winning);
        enter_name = TTF_RenderText_Solid(options_font, "Enter your name below: ", White);
        enter_name_text = SDL_CreateTextureFromSurface(graphics.renderer, enter_name);
        name = TTF_RenderText_Solid(options_font, "Name", White);
        name_text = SDL_CreateTextureFromSurface(graphics.renderer, name);
        time_taken = TTF_RenderText_Solid(options_font, "Time", White);
        time_taken_text = SDL_CreateTextureFromSurface(graphics.renderer, time_taken);
        no_deaths = TTF_RenderText_Solid(options_font, "Deaths", White);
        no_deaths_text = SDL_CreateTextureFromSurface(graphics.renderer, no_deaths);
        leaderboards = TTF_RenderText_Solid(title_font, "Leaderboards", Purple);
        leaderboards_text = SDL_CreateTextureFromSurface(graphics.renderer, leaderboards);
        leaderboards = TTF_RenderText_Solid(options_font, "Leaderboards", White);
        leaderboards_button = SDL_CreateTextureFromSurface(graphics.renderer, leaderboards);

        pause = TTF_RenderText_Solid(title_font, "PAUSED", Purple);
        pause_text = SDL_CreateTextureFromSurface(graphics.renderer, pause);

        //Texture from sprite
        player_sprite = graphics.loadTexture("textures/player.png");
        platform_sprite = graphics.loadTexture("textures/platform.png");
        red_platform_sprite = graphics.loadTexture("textures/platform_red.png");
        blue_platform_sprite = graphics.loadTexture("textures/platform_blue.png");
        red_platform_sprite_disabled = graphics.loadTexture("textures/platform_red_disabled.png");
        blue_platform_sprite_disabled = graphics.loadTexture("textures/platform_blue_disabled.png");
        spike_sprite = graphics.loadTexture("textures/spike.png");
        goal_sprite = graphics.loadTexture("textures/goal.png");
        background = graphics.loadTexture("textures/background.png");
        h2p = graphics.loadTexture("textures/how_to_play.png");

        for (int i = 1; i <= 11; i++){
            std::string link_to_file = "textures/platform (";
            link_to_file += (std::to_string(i).c_str());
            link_to_file += ").png";
            SDL_Texture* tt = graphics.loadTexture(link_to_file.c_str());
            texture_array ta;
            ta.texture = tt;
            platform_type.push_back(ta);
            graphics.renderTexture(platform_type[0].texture, i*10, i*10);
        }
        graphics.renderTexture(platform_type[0].texture, 500, 500);
        for (int i = 1; i <= 4; i++){
            std::string link_to_file = "textures/bridge (";
            link_to_file += (std::to_string(i).c_str());
            link_to_file += ").png";
            SDL_Texture* tt = graphics.loadTexture(link_to_file.c_str());
            texture_array ta;
            ta.texture = tt;
            platform_type.push_back(ta);
        }
}


void SDL_loop(){
    texture_initialization();
    while (true){
        if (game_choice == 0){
            SDL_RenderClear(graphics.renderer);
            game_choice = main_menu_loop();
        }

        else if (game_choice == 2){
            SDL_RenderClear(graphics.renderer);
            game_choice = instructions_loop();
        }

        else if (game_choice == -1){
            break;
        }
        else if (game_choice == 1){
            SDL_RenderClear(graphics.renderer);
            game_choice = main_game_loop();

        }
        else if (game_choice == 3){
            SDL_RenderClear(graphics.renderer);
            game_choice = leaderboard_loop();
        }
    }
    SDL_RenderClear(graphics.renderer);

    graphics.quit();
}

bool level_initialization(Level &lvl, int idx){
    std::string level_file = "level_" + std::to_string(idx) + ".txt";
    Player player1; Goal goal1; Platform platform1; Spike spike1; int posx, posy, type;
    //std::cout << level_file << std::endl;
    std::ifstream file(level_file);
    std::string obj_type;
    if (file.is_open()){
        while (file >> obj_type){
            if (obj_type == "player"){
                file >> posx >> posy;
                player1.create(posx*TILE_MULTIPLIER, posy*TILE_MULTIPLIER);
                lvl.player = player1;
                lvl.init_player = player1;
            }
            else if (obj_type == "goal"){
                file >> posx >> posy;
                goal1.create(posx*TILE_MULTIPLIER, posy*TILE_MULTIPLIER);
                lvl.goal = goal1;
            }
            else if (obj_type == "platform"){
                file >> posx >> posy >> type;
                platform1.create(posx*TILE_MULTIPLIER, posy*TILE_MULTIPLIER, type);
                lvl.platform.push_back(platform1);
            }
            else if (obj_type == "spike"){
                file >> posx >> posy >> type;
                spike1.create(posx*TILE_MULTIPLIER, posy*TILE_MULTIPLIER, type);
                lvl.spike.push_back(spike1);
            }
            else{
                std::cout << "CRITICAL ERROR" << std::endl;
                return 1;
            }
            //std::cout << "ok" << std::endl;
        }
        file.close();
    }
    else{
        std::cout << "Can't find file." << std::endl;
        return 1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    for (int i = 0; i < LEVEL_COUNT; i++){
        level_initialization(level[i], i+1);
    }

    SDL_loop();

    return 0;
}

