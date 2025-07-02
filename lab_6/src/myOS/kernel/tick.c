extern void oneTickUpdateWallClock(void);

void (*tick_hook)(void) = 0;

int tick_number = 0;
void tick(void){
     tick_number++;	

     oneTickUpdateWallClock();

     if(tick_hook) tick_hook();  //user defined   
}
