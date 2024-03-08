
void Setup_Fan_Screen(void);
void Initialize_Setup_Fan_Screen(void) ;
void Update_Setup_Fan_Screen(void);
void Decrease_Fan_Temp();
void Increase_Fan_Temp();
void Exit_Setup_Fan_Temp();
void Do_Save_New_Fan_Temp();
void Setup_Temp_Fan(void);
int get_duty_cycle( float temp, float set_temp);

//  Setup DC Screen   
#define setup_fan_x          15
#define setup_fan_y          60    
#define setup_data_fan_x     40
#define setup_data_fan_y     74