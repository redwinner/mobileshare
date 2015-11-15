/*
 * Copyright (C) shaohongsheng@gmail.com
 *
 *
 */
#include <string.h>
#include <jni.h>
#include <stdio.h>

#include "webui.h"


#include <android/log.h>

#define LOG(x...)   __android_log_print(ANDROID_LOG_INFO,"mobileshare",##x);


#define JNI_PACKAGE_NAME(func)   Java_cn_redwinner_mobileshare_MobileShareJni_##func

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   
 */
jint
JNI_PACKAGE_NAME(startShare)( JNIEnv* env,
                                                  jobject thiz ,
                                                  jstring url)
{
    char *cui[20] = {"share","-ports","8080","-root","/sdcard/",NULL};
    
    const char*  pURL = (*env)->GetStringUTFChars( env, url, 0 );

    char cmd_buffer[1024] = {0};
    int len = strlen(pURL);
    int j = 0;
    int i = 0;
    
    LOG("startShare: %s, enter.",pURL);
    strcpy(cmd_buffer,pURL);

    for(i=0; i< len; i++)
    {
     	if(i==0)
    	{
    	   cui[j++] = &cmd_buffer[0];
    	}

   	if(cmd_buffer[i]==' ')
    	{
    	    cmd_buffer[i] = '\0';
    	    while(cmd_buffer[i+1]==' ')
    	    {
    	    	cmd_buffer[i+1] = '\0';
    	    	i++;
    	    }
    	    cui[j++] = &cmd_buffer[i+1];
    	}
    }
    
    main_ztv_webui_init(j,cui);
    
    return 0;
}

jint
JNI_PACKAGE_NAME(stopShare)( JNIEnv* env,
                                                  jobject thiz)
{
    main_ztv_webui_uninit();
    return 0;
}

jstring
JNI_PACKAGE_NAME(GetNetWorkInformation)( JNIEnv* env,
                                                  jobject thiz)
{
		char netinfo[100] = {0};
		char up[10] = {0};
		char ip[100] = {0};
		char mask[100] = {0};	
		char tmp[100] = {0};			
		FILE *fp_ls = popen("netcfg","r");
		
		if (fp_ls!=NULL) {
		
		 LOG("Net Info Enter");
		 
		 int last_is_up = 0;
		   
		   while(fscanf(fp_ls,"%99s",netinfo)>0)
		   {
			   //read(fp_ls,netinfo,1023);
			   			   
				LOG("Net Info is %s",netinfo);
				
				if(last_is_up)
				{
					strcat(ip,netinfo);
					strcat(ip,",");
				}

				if((netinfo[0]=='U') &&(netinfo[1]=='P'))
				{
                   last_is_up =1;
				}
				else
				{
					last_is_up = 0;
				}
			}
			
			pclose(fp_ls);

		}
		
		LOG("Net Info IP  is %s",ip);

    return (*env)->NewStringUTF(env, ip);
}

jint
JNI_PACKAGE_NAME(setPkgPath)( JNIEnv* env,
                                                  jobject thiz ,
                                                  jstring pkgPath)
{
    const char*  pPath = (*env)->GetStringUTFChars( env, pkgPath, 0 );
    
    ztv_webui_set_share_pkg_path(pPath);
    return 0;
}
