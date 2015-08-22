/*
Copyright (c) 2015 , Ben624

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
THIS APPLICATION CAN BE SHARED ACROSS OWNERS OF THE PEBBLE, BUT CAN NOT BE PUBLISHED PUBLICLY ON THE PEBBLE APP STORE. 

Permission to use, copy, or modify, this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
*/


#include "pebble.h"
#include "colors.h"
	
#define SUNDAY    0
#define MONDAY    1
#define TUESDAY   2
#define WEDNESDAY 3
#define THURSDAY  4
#define FRIDAY    5
#define SATURDAY  6
#define MINUTES   0
#define SECONDS	  1
#define NO		  0
#define YES		  1
#define WHITE	  0
#define BLACK	  1
#define DISABLE	  2
#define NORMAL	  0
#define RELEASE	  1
#define DELAY	  2
#define TIME(h,s) ((h*60) + s)
	
void handle_battery_change(BatteryChargeState b);
void handle_bluetooth_change(bool isconnected);
void readConfig();
void applyConfig(bool callTimerTick);

bool firstrun = true;
bool canPlay = true;
Window *window;
TextLayer *text_date_layer;
TextLayer *text_day_layer;
TextLayer *text_time_layer;
TextLayer *text_time_period_info;
TextLayer *text_battery_info;
TextLayer *text_bluetooth_info;
static GBitmap *bluetooth_on_bitmap;
static BitmapLayer *bluetooth_on_layer;

Layer *top_line_layer;
Layer *bottom_line_layer;

struct classNames {
	char className[20];

};
struct appearanceInfo {
	
	int schoolDay;
	int clockStyle;
	int textColor;
	int bluetooth;
	int alarm;
	int battery;
	int alarmRung;
	int break_mode;
	char dateColor[10];
	char lineColor[10];
	char timeColor[10];
	char batColor[10];
	char blueColor[10];
	char backColor[10];
	char periodColor[10];
	
};
struct appearanceInfo aInfo;
struct classNames cNames[8];

struct periodInfo {
	int period;
	int start;
	int end;
};

//Don't mess with anything above here. 



/*

Enter the times for the normal bell schedule below.

The structs below are in the following format. 
{period number, START TIME(HOURS,MINUTES), END TIME(HOURS,MINUTES)}
**NOTE THAT THE TIMES ARE IN 24HR
**Period Names are set in the app configuration page. 
*/
struct periodInfo normalInfo[] = { 
    {1, TIME(8,35),  TIME(9,30)},
	{2, TIME(8,35),  TIME(9,35)},
	{3, TIME(9,48),  TIME(10,48)},
	{4, TIME(10,54),  TIME(11,54)},
	{5, TIME(11,10),  TIME(12,00)},
	{6, TIME(12,05),  TIME(12,50)},
	{7, TIME(12,55), TIME(13,40)},
	{8, TIME(13,45), TIME(14,30)}
};
#define INFO_SIZE (sizeof(normalInfo)/sizeof(struct periodInfo))
	

#define NORMAL_START TIME(7,45)//Enter Normal Day Start Time
#define NORMAL_END   TIME(14,30)//Enter Normal Day End Time

	
//The Delay info and early release info follow the same format as what was stated above. 
//If your school does  not have either delay or early release, just leave it alone and don't select it in the config page. 
struct periodInfo delayInfo[] = { 
	{1, TIME(9,45),  TIME(10,15)},
	{2, TIME(10,20),  TIME(10,50)},
	{3, TIME(10,55),  TIME(11,30)},
	{5, TIME(11,35),  TIME(12,10)},
	{4, TIME(12,15),  TIME(12,45)},
	{6, TIME(12,50),  TIME(13,20)},
	{7, TIME(13,25), TIME(13,55)},
	{8, TIME(14,00), TIME(14,30)}
};
#define DELAY_START  TIME(9,45)//Enter Delay Day Start Time
#define DELAY_END    TIME(14,30) //Enter Delay Day Start Time

struct periodInfo earlyRelInfo[] = {
	{1, TIME(7,45),  TIME(8,10)},
	{2, TIME(8,15),  TIME(8,40)},
	{3, TIME(8,45),  TIME(9,10)},
	{4, TIME(9,15),  TIME(9,40)},
	{6, TIME(9,45),  TIME(10,10)},
	{7, TIME(10,15),  TIME(10,40)},
	{8, TIME(10,45), TIME(11,10)},
	{5, TIME(11,15), TIME(11,50)}
};
#define EARLY_REL_START TIME(7,45) //Enter Early Release Day Start Time
#define EARLY_REL_END   TIME(11,50) //Enter Early Release Day Start Time

struct periodInfo *pinfo = normalInfo;
static int school_start = NORMAL_START;
static int school_end   = NORMAL_END;





//Animation(Boot Animation) Stopped Handler 
void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy((PropertyAnimation*) anim);
}
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    	PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
     
    	animation_set_duration((Animation*) anim, duration);
    	animation_set_delay((Animation*) anim, delay);
     
		AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) on_animation_stopped
        };
    	animation_set_handlers((Animation*) anim, handlers, NULL);
     
    	animation_schedule((Animation*) anim);
	
}

//Top line across screen below date. 
void top_line_layer_update_callback(Layer *layer, GContext* ctx) {
	#ifdef PBL_COLOR
	graphics_context_set_fill_color(ctx, GColorFromHEX(findColor(aInfo.lineColor)));	
	#else
    if(aInfo.textColor == WHITE) graphics_context_set_fill_color(ctx, GColorWhite);
	else graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
//Bottom line across screen below time.
void bottom_line_layer_update_callback(Layer *layer, GContext* ctx) {
	#ifdef PBL_COLOR
	graphics_context_set_fill_color(ctx, GColorFromHEX(findColor(aInfo.lineColor)));	
	#else
	if(aInfo.textColor == WHITE) graphics_context_set_fill_color(ctx, GColorWhite);
	else graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
//Boot animation... change this to change the boot animation. 
void boot_animation(){
	GRect dayStart  = GRect(350, 34, 144 - 6, 30);
	GRect dayFinish = GRect(2, 34, 144 - 6, 30);
    animate_layer(text_layer_get_layer(text_day_layer), &dayStart, &dayFinish, 1600, 20);
	GRect dateStart =  GRect(350, 54, 144 - 6, 30);
	GRect dateFinish = GRect(2, 54, 144 - 6, 30);
    animate_layer(text_layer_get_layer(text_date_layer), &dateStart, &dateFinish, 1600, 10);
	
	GRect clockStart = GRect(-200,70, 144 - 7, 55);
	GRect clockFinish =GRect(2, 70, 144 - 7, 55);
    animate_layer(text_layer_get_layer(text_time_layer), &clockStart, &clockFinish, 1600,20);
	
	GRect periodStart = GRect(2, 300, 144 - 7, 90);
	GRect periodFinish =GRect(2, 108, 144 - 7, 90);
    animate_layer(text_layer_get_layer(text_time_period_info), &periodStart, &periodFinish, 1400, 10);
	
	GRect topLineStart = GRect(200, 76, 180, 2);
	GRect topLineFinish =GRect(0, 76, 180, 2);
    animate_layer(top_line_layer, &topLineStart, &topLineFinish, 1400, 10);
	
	GRect bottomLineStart = GRect(-200, 108, 180, 2);
	GRect bottomLineFinish =GRect(0, 108, 180, 2);
    animate_layer(bottom_line_layer, &bottomLineStart, &bottomLineFinish, 1400, 10);
	
	GRect bluetoothStart = GRect(-10, -100, 48, 48);
	GRect bluetoothFinish =GRect(-10, -10, 48, 48);
    animate_layer(bitmap_layer_get_layer(bluetooth_on_layer), &bluetoothStart, &bluetoothFinish, 1400, 10);
	
	
	GRect batteryStart = GRect(70, -100, 144 - 7, 90);
	GRect batteryFinish =GRect(70, 0, 144 - 7, 90);
    animate_layer(text_layer_get_layer(text_battery_info), &batteryStart, &batteryFinish, 1400, 10);
}



//Tick handler... Don't mess with this unless you know what to do :) 

void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (firstrun) {
		firstrun = false;
		}
	static char outline[81];
	static bool between_classes = false;
	static int next_class_at = 0;
	static int time_to_school = 0;
	static char time_text[] = "00:00:00";
	static char date_text[] = "00/00/0000";
	static char day_text[] = "Xxxxxxxxx";

	char *time_format;

	int hours = tick_time->tm_hour;
	int minutes = tick_time->tm_min;
	int seconds = tick_time->tm_sec;
	int day = tick_time->tm_wday;
	int now = (hours * 60) + minutes + 1;
	if (clock_is_24h_style()) {
		if(aInfo.clockStyle == MINUTES)
		time_format = "%R";
		else
		time_format = "%T";
	} else {
		if(aInfo.clockStyle == MINUTES)
		time_format = "%I:%M";
		else
		time_format = "%I:%M:%S";
	}
	strftime(time_text, sizeof(time_text), time_format, tick_time);
	
	if (!clock_is_24h_style() && (time_text[0] == '0')) {
		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}

	text_layer_set_text(text_time_layer, time_text);
	if (seconds == 0 || hours == 0 || !firstrun) {
		//UPDATES DATE
		strftime(day_text, sizeof(day_text), "%A", tick_time);
		strftime(date_text, sizeof(date_text), "%D", tick_time);
		text_layer_set_text(text_date_layer, date_text);
		text_layer_set_text(text_day_layer, day_text);
	}
	
	if(aInfo.break_mode == 1){
		text_layer_set_text(text_time_period_info, "Break");

	}else{

	if (between_classes) {
		if(canPlay){
//		start_animate_next_class_countdown();
		canPlay = false; 
		}
		if ( next_class_at < now) {
			between_classes = false;
			return;
		}
		if(aInfo.clockStyle == MINUTES)
		snprintf(outline, 80, "Next Class In:\n%d Minutes",next_class_at - now);
		else
		snprintf(outline, 80, "Next Class In:\n%02d:%02d",next_class_at - now,60-seconds);
//60-seconds
		text_layer_set_text(text_time_period_info, outline);

		if (now == next_class_at){
			between_classes = false;
			canPlay = true;
		}
		return;
	}

	if (day == SATURDAY || day == SUNDAY  || (day == FRIDAY && now > school_end)) {
	text_layer_set_text(text_time_period_info, "Weekend");

	}
	else if (now >= school_start && now <= school_end) {
		text_layer_set_text(text_time_period_info, outline);
		for (unsigned int i = 0; i < INFO_SIZE; i++) {
			if ((now >= pinfo[i].start) && (now <= pinfo[i].end)) {
					if(aInfo.clockStyle == MINUTES)
					snprintf(outline, 80, "%s:\n%d minutes left\n", cNames[i].className, pinfo[i].end - now);
					else
					snprintf(outline, 80, "%s:\n%02d:%02d", cNames[i].className, pinfo[i].end - now,60-seconds);
					break;
				
			} 
			else if ((now > pinfo[i].end) && (now <= pinfo[i].end+5)) {
				between_classes = true;
				next_class_at = pinfo[i].end + 5;
				if(aInfo.clockStyle == MINUTES)
				snprintf(outline, 80, "Next Class In:\n%d Minutes",next_class_at - now);
				else
				snprintf(outline, 80, "Next Class In:\n%02d:%02d",next_class_at - now,60-seconds);	
				text_layer_set_text(text_time_period_info, outline);
				break; 
			}
		}
		text_layer_set_text(text_time_period_info, outline);
	} else {
		if (now >= school_end) {
			time_to_school = (24 * 60) - now + ((7 * 60) + 25);
		} else if (now <= school_start) {
			time_to_school = school_start - now;
		}
		int afterhours = time_to_school / 60;
		int afterminutes = time_to_school % 60;
		if (afterhours > 0) {
			if(aInfo.clockStyle == MINUTES)
			snprintf(outline, 80, "School in:\n%d hours\n%d minutes",afterhours, afterminutes);
			else
			snprintf(outline, 80, "School in:\n%d:%02d:%02d",afterhours, afterminutes,60-seconds);
		} else {
			if(aInfo.clockStyle == MINUTES)
			snprintf(outline,80,"School in:\n%d minutes", afterminutes);
			else
			snprintf(outline,80,"School in:\n%02d:%02d", afterminutes,60-seconds);

		}
		if (day == FRIDAY && now >= school_end)
			text_layer_set_text(text_time_period_info, "School's out!");
		else
			text_layer_set_text(text_time_period_info, outline);
	}
}
}

//Bluetooth Connection loss vibration. Change this if you want a different vibration when bluetooth lost. 
static const uint32_t segments[] = { 400, 100, 400,100,400};
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};

//Bluetooth handler. There shouldn't be a need to mess with this. 
void handle_bluetooth_change(bool isconnected){

	#ifdef PBL_COLOR
		if(isconnected){
		if((strcmp(aInfo.blueColor, "FFFFFF") == 0))
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED_WHITE));
		else
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED_BLACK));
		aInfo.alarmRung = NO;
	}else{
		if((strcmp(aInfo.blueColor, "FFFFFF") == 0))
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOT_CONNECTED_WHITE));
		else
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOT_CONNECTED_BLACK));
	}
	
	if(aInfo.alarm == YES && !isconnected && aInfo.alarmRung==NO){
		vibes_enqueue_custom_pattern(pat);
	if((strcmp(aInfo.blueColor, "FFFFFF") == 0))
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_WHITE));
		else
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_BLACK));
		aInfo.alarmRung = YES;
	}
	#else		
    if(isconnected){
		if(aInfo.textColor == WHITE){
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssign); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED_WHITE));
		}else{
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssignInverted); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED_WHITE));
		}
		aInfo.alarmRung = NO;
	}
	else {
		if(aInfo.textColor == WHITE){
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssign); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOT_CONNECTED_WHITE));
		}
		else{
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssignInverted); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOT_CONNECTED_WHITE));
		}	
	}

	if(aInfo.alarm == YES && !isconnected && aInfo.alarmRung==NO){
		vibes_enqueue_custom_pattern(pat);
	if(aInfo.textColor == WHITE){
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssign); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_WHITE));
	}else{
			bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssignInverted); 
			bitmap_layer_set_bitmap(bluetooth_on_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_WHITE));	}
		aInfo.alarmRung = YES;
	}
	
	#endif
	if(aInfo.bluetooth == NO)
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_on_layer), true);
	else
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_on_layer), false);

}

//Battery handler. There shouldn't be a need to mess with this. 

void handle_battery_change(BatteryChargeState b) {
    static char outline[40];
       if(b.is_charging)
        text_layer_set_text(text_battery_info, "       Charging");
     else if(b.charge_percent == 100)
        text_layer_set_text(text_battery_info, "        Charged");
	   else{
	    snprintf(outline,sizeof(outline),"               %d%% ",b.charge_percent);
		text_layer_set_text(text_battery_info,outline);

	}
	if(aInfo.battery == NO)
	text_layer_set_text(text_battery_info," ");
	   
    }
        

//Configuration handlers start
void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}


void updateColors(){
	#ifdef PBL_COLOR
		window_set_background_color(window, GColorFromHEX(findColor(aInfo.backColor)));
		text_layer_set_text_color(text_date_layer, GColorFromHEX(findColor(aInfo.dateColor)));
		text_layer_set_text_color(text_day_layer, GColorFromHEX(findColor(aInfo.dateColor)));
		text_layer_set_text_color(text_time_layer, GColorFromHEX(findColor(aInfo.timeColor)));
		text_layer_set_text_color(text_time_period_info, GColorFromHEX(findColor(aInfo.periodColor)));
		text_layer_set_text_color(text_battery_info, GColorFromHEX(findColor(aInfo.batColor)));
		text_layer_set_text_color(text_bluetooth_info, GColorFromHEX(findColor(aInfo.blueColor)));	
		#else
	if(aInfo.textColor == WHITE){
		window_set_background_color(window, GColorBlack);
		text_layer_set_text_color(text_date_layer, GColorWhite);
		text_layer_set_text_color(text_day_layer, GColorWhite);
		text_layer_set_text_color(text_time_layer, GColorWhite);
		text_layer_set_text_color(text_time_period_info, GColorWhite);
		text_layer_set_text_color(text_battery_info, GColorWhite);
		text_layer_set_text_color(text_bluetooth_info, GColorWhite);

	}else{
		window_set_background_color(window, GColorWhite);
		text_layer_set_text_color(text_date_layer, GColorBlack);
		text_layer_set_text_color(text_day_layer, GColorBlack);
		text_layer_set_text_color(text_time_layer, GColorBlack);
		text_layer_set_text_color(text_time_period_info, GColorBlack);
		text_layer_set_text_color(text_battery_info, GColorBlack);
		text_layer_set_text_color(text_bluetooth_info, GColorBlack);


	}
	#endif
	text_layer_set_background_color(text_date_layer, GColorClear);
	text_layer_set_background_color(text_day_layer, GColorClear);
    text_layer_set_background_color(text_time_layer, GColorClear);
	text_layer_set_background_color(text_time_period_info, GColorClear);
    text_layer_set_background_color(text_bluetooth_info, GColorClear);
    text_layer_set_background_color(text_battery_info, GColorClear);
	layer_set_update_proc(top_line_layer, top_line_layer_update_callback);
    layer_set_update_proc(bottom_line_layer, bottom_line_layer_update_callback);
}

void updateBatBlue(){
	battery_state_service_subscribe(handle_battery_change);
    handle_battery_change(battery_state_service_peek());
    bluetooth_connection_service_subscribe(handle_bluetooth_change);
    handle_bluetooth_change(bluetooth_connection_service_peek());
}

void updateClock(){
 if (aInfo.clockStyle == MINUTES){
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    }
    else{
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    }
	
	
if(aInfo.clockStyle == MINUTES){
text_layer_set_font(text_time_period_info,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_17)));	
}else{
	text_layer_set_font(text_time_period_info,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_REGULAR_SUBSET_22)));	
}
}

void updateSchoolMode(){
	switch (aInfo.schoolDay) {
		case NORMAL:
			pinfo        = normalInfo;
			school_start = NORMAL_START;
	 		school_end   = NORMAL_END;
			break;
		case DELAY:
			pinfo        = delayInfo;
			school_start = DELAY_START;
	 		school_end   = DELAY_END;	
			break;
		case RELEASE:
			pinfo = earlyRelInfo;
			school_start = EARLY_REL_START;
			school_end = EARLY_REL_END;
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "MODE Error");
			break;
	}
	
}

void in_received_handler(DictionaryIterator *received, void *context) {
	for(int x = 0;x<8;x++){
        Tuple *Name = dict_find(received, 21+x);
		if(strcmp(Name->value->cstring, "null") == 0){
		char temp[40];
		snprintf(temp,sizeof(temp),"Period %d",x+1);
	    strcpy(cNames[x].className,temp);
		}else
		strcpy(cNames[x].className,Name->value->cstring);
}
	  Tuple *sDay = dict_find(received, 29);
      Tuple *wMode = dict_find(received, 30);
	  Tuple *tColorc = dict_find(received, 48);
	  Tuple *blue = dict_find(received, 32);
	  Tuple *bluea = dict_find(received, 36);
	  Tuple *bat = dict_find(received, 33);
	  Tuple *brk = dict_find(received, 49);
	
	  Tuple *dcolor = dict_find(received, 41);
	  Tuple *lcolor = dict_find(received, 42);
	  Tuple *tcolor = dict_find(received,43);
	  Tuple *batcolor = dict_find(received, 44);
	  Tuple *bluecolor = dict_find(received, 45);
	  Tuple *backcolor = dict_find(received, 46);
	  Tuple *periodcolor = dict_find(received, 47);
	
	   if(sDay->value->cstring[0] == 'n')aInfo.schoolDay = 0;
       else if(sDay->value->cstring[0] == 'e') aInfo.schoolDay = 1;
	   else aInfo.schoolDay = 2;
	 
	    if(wMode->value->cstring[0] == 'm') aInfo.clockStyle = MINUTES; 
	  	else aInfo.clockStyle = SECONDS;
	
		if(tColorc->value->cstring[0] == 'w') aInfo.textColor = WHITE; 
	  	else aInfo.textColor = BLACK;
	
		if(blue->value->cstring[0] == 'y') aInfo.bluetooth = YES; 
	  	else aInfo.bluetooth = NO;
	
	    if(bluea->value->cstring[0] == 'y') aInfo.alarm = YES; 
	  	else aInfo.alarm = NO;
	
	    if(bat->value->cstring[0] == 'y') aInfo.battery = YES; 
	  	else aInfo.battery = NO;
	
		if(brk->value->cstring[0] == 'y') aInfo.break_mode = YES; 
	  	else aInfo.break_mode = NO;
	    
      strcpy(aInfo.backColor,backcolor->value->cstring);
      strcpy(aInfo.dateColor,dcolor->value->cstring);
	  strcpy(aInfo.lineColor,lcolor->value->cstring);
      strcpy(aInfo.timeColor,tcolor->value->cstring);
      strcpy(aInfo.batColor,batcolor->value->cstring);
      strcpy(aInfo.blueColor,bluecolor->value->cstring);
      strcpy(aInfo.periodColor,periodcolor->value->cstring);
	
	
	updateColors();
	updateBatBlue();
	updateSchoolMode();
	updateClock();
}

void readConfig() {
	if(persist_exists(34)){
		persist_read_data(34,&aInfo,sizeof(aInfo));
	}else{
		aInfo.bluetooth = YES;
		aInfo.battery = YES;
		#ifdef PBL_COLOR
      strcpy(aInfo.backColor,"000000");
      strcpy(aInfo.dateColor,"FFFFFF");
	  strcpy(aInfo.lineColor,"FFFFFF");
      strcpy(aInfo.timeColor,"FFFFFF");
      strcpy(aInfo.batColor,"FFFFFF");
      strcpy(aInfo.blueColor,"FFFFFF");
      strcpy(aInfo.periodColor,"FFFFFF");
		#else
		aInfo.textColor = WHITE;
		#endif
	}
	
	if(persist_exists(35)){
        persist_read_data(35,cNames,sizeof(cNames));
    }else{
		for(int x = 0;x<8;x++){
		char temp[40];
		snprintf(temp,sizeof(temp),"Period %d",x+1);
	    strcpy(cNames[x].className,temp);

}
	}
	
	
//Config handlers end
}


//Appmessage for receiving js code. 
static void app_message_init(void) {
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_open(app_message_inbox_size_maximum(),app_message_inbox_size_maximum());
    
}

//Watchface close handler 
void handle_deinit(void) {
	persist_write_data(34, &aInfo, sizeof(aInfo));
    persist_write_data(35, cNames,sizeof(cNames));// Saves class names
    tick_timer_service_unsubscribe();
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Deinit Saved!");
    
    
}


// Initial Init handler 
void handle_init(void) {
    window = window_create();
    window_stack_push(window, true);
    window_set_background_color(window, GColorWhite);
    
    Layer *window_layer = window_get_root_layer(window);
    app_message_init();
    readConfig();
    //Date
    text_date_layer = text_layer_create(GRect(350, 54, 144 - 6, 30));
    text_layer_set_text_color(text_date_layer, GColorBlack);
    text_layer_set_background_color(text_date_layer, GColorClear);
    text_layer_set_font(text_date_layer,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_19)));
    layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
    
    //DAY
    text_day_layer = text_layer_create(GRect(350, 34, 144 - 6, 30));
   text_layer_set_text_color(text_day_layer, GColorBlack);
    text_layer_set_background_color(text_day_layer, GColorClear);
    text_layer_set_font(text_day_layer,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_17)));
    layer_add_child(window_layer, text_layer_get_layer(text_day_layer));
	
    //time
    text_time_layer = text_layer_create(GRect(-200,70, 144 - 7, 55));
    text_layer_set_text_color(text_time_layer, GColorBlack);
    text_layer_set_background_color(text_time_layer, GColorClear);
    text_layer_set_font(text_time_layer,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_36)));
    layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	
    //Period
    text_time_period_info = text_layer_create(GRect(2, 300, 144 - 7, 90));
    text_layer_set_text_color(text_time_period_info, GColorBlack);
   text_layer_set_background_color(text_time_period_info, GColorClear);
   text_layer_set_font(text_time_period_info,
    fonts_load_custom_font(
    resource_get_handle(
    RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_19)));
    layer_add_child(window_layer, text_layer_get_layer(text_time_period_info));
    //Battery
    text_battery_info = text_layer_create(GRect(70, -100, 144 - 7, 90));
    text_layer_set_text_color(text_battery_info, GColorBlack);
    text_layer_set_background_color(text_battery_info, GColorClear);
    text_layer_set_font(text_battery_info,
    fonts_get_system_font(FONT_KEY_GOTHIC_18));
    layer_add_child(window_layer, text_layer_get_layer(text_battery_info));
    
    //BLUETOOTH
    text_bluetooth_info = text_layer_create(GRect(-10, -100, 48, 48));
    text_layer_set_text_color(text_bluetooth_info, GColorBlack);
    text_layer_set_background_color(text_bluetooth_info, GColorClear);
    text_layer_set_font(text_bluetooth_info,
    fonts_get_system_font(FONT_KEY_GOTHIC_18));
  //  layer_add_child(window_layer, text_layer_get_layer(text_bluetooth_info));
    
    GRect top_line_frame = GRect(200, 76, 180, 2);
    GRect bottom_line_frame = GRect(-200, 108, 180, 2);
    top_line_layer = layer_create(top_line_frame);
    bottom_line_layer = layer_create(bottom_line_frame);
    layer_set_update_proc(top_line_layer, top_line_layer_update_callback);
    layer_set_update_proc(bottom_line_layer, bottom_line_layer_update_callback);
    layer_add_child(window_layer, top_line_layer);
    layer_add_child(window_layer, bottom_line_layer);
	//BITMAP START  RESOURCE_ID_IMAGE_BLUE_ON
	bluetooth_on_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED_BLACK);//
	bluetooth_on_layer = bitmap_layer_create(GRect(-10, -10, 48, 48));
	#ifdef PBL_COLOR
		  bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpSet);
	#else
		  bitmap_layer_set_compositing_mode(bluetooth_on_layer, GCompOpAssignInverted); 
	#endif
bitmap_layer_set_bitmap(bluetooth_on_layer, bluetooth_on_bitmap);
layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bluetooth_on_layer));
	//BITMAP STOP
    updateColors();
	updateBatBlue();
	updateClock();
	boot_animation();
    if (aInfo.clockStyle == MINUTES){
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    }
    else{
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    }
}

//Main
int main(void) {
    handle_init();
    
    app_event_loop();
    
    handle_deinit();
}
