// .cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>

#include "webui.h"

int main(int argc, char* argv[])
{
    static	char commandid[16];

	main_ztv_webui_init(argc,argv);
	ztv_webui_set_share_pkg_path("mobileshare.apk");

	while (1)
	{
		printf("**********MobileShare *********\n\n");
		printf("Please input a number:\n");
		printf("q Quit.\n");

		scanf("%s",commandid);
		switch(commandid[0])
		{
		case 'q':
			{  
				printf("Receive command Quit\n");
				main_ztv_webui_uninit();
				return 0;
			}
			break;
		default:
			{}
		}
	}

	return 0;
}

