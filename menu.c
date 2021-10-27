#include "globals.h"

static int menu_options_alloc;


void initMenu()
{
	menu_options = NULL;
	menu_option_cnt = 0;
	menu_options_alloc = 0;
}




void addMenuOption(u_char *mesg, uint32_t len)
{
	char *strm;

	/* If we've received an option number thats less than the current
	   count AND we're not still receiving the menu then reset */
	if (isdigit(mesg[0]) &&
	    (mesg[0] - '0') < menu_option_cnt && !FLAGISSET(FLAG_IN_MENU))
	{
		clearMenu();
	}

	/* Allocate new memory in blocks. Could do 1 by 1 but thats not very
	   efficient */
	if (menu_option_cnt >= menu_options_alloc)
	{
		menu_options_alloc += ALLOC_BLOCK;
		menu_options = (char **)realloc(
			menu_options,menu_options_alloc * sizeof(char **));
		assert(menu_options);
	}
	strm = (char *)malloc(len+1);
	assert(strm);
	memcpy(strm,mesg+2,len);
	strm[len] = 0;
	menu_options[menu_option_cnt] = strm;
	++menu_option_cnt;
}




void setMenuSelection()
{
	if (menu_selection)
	{
		free(menu_selection);
		menu_selection = NULL;
	}
	if (menu_cursor_pos != -1 && menu_cursor_pos < menu_option_cnt)
		menu_selection = strdup(menu_options[menu_cursor_pos]);
}




/*** Wanted to have the menu selection as another cursor pointing to the
     selected option but it was too complex figuring out when the menus
     changed so just using a string copy of the option ***/
void printMenuList()
{
	int i;

	if (menu_option_cnt)
	{
		puts("\n*** Menu list ***\n");
		for(i=0;i < menu_option_cnt;++i)
		{
			printf("%s %-2d: %s\n",
				i == menu_cursor_pos ? "->" : "  ",
				i,menu_options[i]);
		}
		putchar('\n');
	}
	else puts("No menu list stored.");
	printf("Calculated cursor pos : %d\n",menu_cursor_pos);
	printMenuSelection();
}




void printMenuSelection()
{
	printf("Current menu selection: %s\n",
		menu_selection ? menu_selection : "<not set>");
}




void clearMenu()
{
	int i;
	for(i=0;i < menu_option_cnt;++i) free(menu_options[i]);
	free(menu_options);
	initMenu();
}
