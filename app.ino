
class Menu{
	public:
		// Инциализация меню


		void init(){
			Serial.println("Menu init: begin");
			menuInit();
			cm_id = 0;
			cur_pos = 0;

			add2childrens(0,{1,2,3,4,5});

			Serial.println("Menu init: end");
		}
		// Рисование меню
		void draw(){

			if(!hasUpdate) return;

			Serial.println(mi[cm_id].title);

			ch_sz = countItems();

			for(i = 0; i <= ch_sz; i++){
				setData(mi[cm_id].child[i]);

				if(i == cur_pos) Serial.print(" + ");
				else Serial.print("   ");
				Serial.println(cm.title);
			}

			hasUpdate = false;
		}

		void up(){
			cur_pos--;
			if(cur_pos < 0) cur_pos = ch_sz;
		}

		void down(){
			cur_pos++;
			if(cur_pos > ch_sz) cur_pos = 0;
		}

		void ok(){
			cm_id = mi[cm_id].child[cur_pos];
			cur_pos = 0;
		}

		void back(){
			byte old_id = cm_id;
			cm_id = mi[cm_id].parent;
			for(i = 0; i < max_childrens; i++){
				if(mi[cm_id].child[i] == old_id){
					cur_pos = mi[cm_id].child[i]-1;
					break;
				}
			}
		}

		void info(){
			// cout << "--- INFO ---" << endl;
			// cout << "cm_id: " << cm_id << endl;
			// cout << "cur_pos: " << cur_pos << endl;
			// cout << "ch_sz: " << ch_sz << endl;
			// cout << "--- INFO ---" << endl;
			// system("pause");
		}

		bool hasUpdate = true;

	private:
		void add2childrens(byte id, int insert[16]){
	for(int i = 0; i < 16; i++){
		if(insert[16] != 0) mi[id].child[i] = insert[i];
		else break;
	}
}
		/*
			i, k - счётчики
			cur_pos - позиция курсора
			cm_id - id текущего меню
			max_childrens - макс. к-во потомков в меню.
		*/
		int i, k, cur_pos, cm_id, ch_sz;
		static const byte max_childrens = 16;
		// Струкрута меню
		struct menuItems{
			char* title;
			byte type;
			byte parent;
			byte child[max_childrens];
		} mi[6], cm;
		// Инциализация меню
		void menuInit(){
			mi[0].title = "MEIN MENU";
			mi[0].type = 1;
			mi[0].parent = 0;
			// mi[0].child[0] = 1;
			// mi[0].child[1] = 2;
			// mi[0].child[2] = 3;
			// mi[0].child[3] = 4;
			// mi[0].child[4] = 5;

			mi[1].title = "Menu_1";
			mi[1].type = 1;
			mi[1].parent = 0;
			// mi[1].child[0] = 1;
			// mi[1].child[1] = 2;
			// mi[1].child[2] = 2;

			mi[2].title = "Menu_2";
			mi[2].type = 1;
			mi[2].parent = 0;
			// mi[2].child[0] = 1;
			// mi[2].child[1] = 2;
			// mi[2].child[2] = 3;
			// mi[2].child[3] = 5;

			mi[3].title = "Menu_3";
			mi[3].type = 1;
			mi[3].parent = 0;
			// mi[3].child[0] = 1;
			// mi[3].child[1] = 2;

			mi[4].title = "Menu_4";
			mi[4].type = 1;
			mi[4].parent = 0;
			// mi[4].child[0] = 1;
			// mi[4].child[1] = 2;

			mi[5].title = "Menu_5";
			mi[5].type = 1;
			mi[5].parent = 0;
			// mi[5].child[0] = 1;
			// mi[5].child[1] = 2;
		}
		// Установка значений текущего меню в переменные
		void setData(int id){
			cm.title = mi[id].title;
			cm.type = mi[id].type;
			cm.parent = mi[id].parent;
		}
		// Подсчитать к-во эллементов в массиве потомков
		int countItems(){
			int countIt = 0;
			for(i = 0; i < max_childrens; i++){
				if(mi[cm_id].child[i] != 0) countIt++;
				else break;
			}
			return countIt-1;
		}
};

// void input2Array(int& var_arr[16], int new_arr[16]){
// 	for(int i = 0; i < sizeof(new_arr)/sizeof(int); i++){
// 		var_arr[i] = new_arr[i];
// 	}
// }

Menu menu;

void setup() {
	Serial.begin(9600);
	menu.init();
}

int serial_input;

void loop(){
	menu.draw();
	if (Serial.available() > 0){
		serial_input = Serial.read();
		if(serial_input == 56 || serial_input == 50 || serial_input == 53 || serial_input == 57) menu.hasUpdate = true;
		switch(serial_input){
			case 56: menu.up(); break;
			case 50: menu.down(); break;
			case 53: menu.ok(); break;
			case 57: menu.back(); break;
		}
	}
}