
 
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS	/* Disable deprecation warning in VS2005 */
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "webui.h"
#include "mongoose.h"

#ifdef _WIN32
#include <windows.h>
#include <winsvc.h>
#define DIRSEP			'\\'
#define	snprintf		_snprintf
#if !defined(__LCC__)
#define	strdup(x)		_strdup(x)
#endif /* !MINGW */
#define	sleep(x)		Sleep((x) * 1000)
#else
#include <sys/wait.h>
#include <unistd.h>		/* For pause() */
#define DIRSEP '/'
#endif /* _WIN32 */

static int exit_flag;	                /* Program termination flag	*/

static FP_OPENURL_CB __fp_openurl = NULL;
static void *__ref = NULL; 

static char g_pkgPath[1024] = {0};

#if !defined(CONFIG_FILE)
#define	CONFIG_FILE		"mongoose.conf"
#endif /* !CONFIG_FILE */

static void
signal_handler(int sig_num)
{
#if !defined(_WIN32)
	if (sig_num == SIGCHLD) {
		do {
		} while (waitpid(-1, &sig_num, WNOHANG) > 0);
	} else
#endif /* !_WIN32 */
	{
		exit_flag = sig_num;
	}
}

/*
 * Show usage string and exit.
 */
static void
show_usage_and_exit(void)
{
	mg_show_usage_string(stderr);
	exit(EXIT_FAILURE);
}

/*
 * Edit the passwords file.
 */
static int
mg_edit_passwords(const char *fname, const char *domain,
		const char *user, const char *pass)
{
	struct mg_context	*ctx;
	int			retval;

	ctx = mg_start();
	(void) mg_set_option(ctx, "auth_realm", domain);
	retval = mg_modify_passwords_file(ctx, fname, user, pass);
	mg_stop(ctx);

	return (retval);
}

static void
process_command_line_arguments(struct mg_context *ctx, char *argv[])
{
	const char	*config_file = CONFIG_FILE;
	char		line[512], opt[512], *vals[100],
				val[512], path[FILENAME_MAX], *p;
	FILE		*fp;
	size_t		i, line_no = 0;

	/* First find out, which config file to open */
	for (i = 1; argv[i] != NULL && argv[i][0] == '-'; i += 2)
		if (argv[i + 1] == NULL)
			show_usage_and_exit();

	if (argv[i] != NULL && argv[i + 1] != NULL) {
		/* More than one non-option arguments are given */
		show_usage_and_exit();
	} else if (argv[i] != NULL) {
		/* Just one non-option argument is given, this is config file */
		config_file = argv[i];
	} else {
		/* No config file specified. Look for one where binary lives */
		if ((p = strrchr(argv[0], DIRSEP)) != 0) {
			(void) snprintf(path, sizeof(path), "%.*s%s",
			    (int) (p - argv[0]) + 1, argv[0], config_file);
			config_file = path;
		}
	}

	fp = fopen(config_file, "r");

	/* If config file was set in command line and open failed, exit */
	if (fp == NULL && argv[i] != NULL) {
		(void) fprintf(stderr, "cannot open config file %s: %s\n",
		    config_file, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Reset temporary value holders */
	(void) memset(vals, 0, sizeof(vals));

	if (fp != NULL) {
		(void) printf("Loading config file %s\n", config_file);

		/* Loop over the lines in config file */
		while (fgets(line, sizeof(line), fp) != NULL) {

			line_no++;

			/* Ignore empty lines and comments */
			if (line[0] == '#' || line[0] == '\n')
				continue;

			if (sscanf(line, "%s %[^\r\n#]", opt, val) != 2) {
				fprintf(stderr, "%s: line %d is invalid\n",
				    config_file, (int) line_no);
				exit(EXIT_FAILURE);
			}
			if (mg_set_option(ctx, opt, val) != 1)
				exit(EXIT_FAILURE);
		}

		(void) fclose(fp);
	}

	/* Now pass through the command line options */
	for (i = 1; argv[i] != NULL && argv[i][0] == '-'; i += 2)
		if (mg_set_option(ctx, &argv[i][1], argv[i + 1]) != 1)
			exit(EXIT_FAILURE);
}


static void
login_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
	char		*name, *pass;

	name = mg_get_var(conn, "UserId");
	pass = mg_get_var(conn, "PassWord");

	/*
	 * Here user name and password must be checked against some
	 * database - this is step 2 from the algorithm described above.
	 * This is an example, so hardcode name and password to be
	 * admin/admin, and if this is so, set "allow=yes" cookie and
	 * redirect back to the page where we have been redirected to login.
	 */
	if (name != NULL && pass != NULL ) {
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "Name is %s<br>"
            "password is %s<br>",name,pass);
	} else {
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "Please login (enter admin/admin to pass)<br>"
		    "<form method=post>"
		    "Name: <input type=text name=name></input><br/>"
		    "Password: <input type=password name=pass></input><br/>"
		    "<input type=submit value=Login></input>"
		    "</form>");
	}

	if (name != NULL)
		mg_free(name);
	if (pass != NULL)
		mg_free(pass);

	return ;
}

static void
ztv_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
	char		*open_url;

	open_url = mg_get_var(conn, "open_url");

	/*
	 * Here user name and password must be checked against some
	 * database - this is step 2 from the algorithm described above.
	 * This is an example, so hardcode name and password to be
	 * admin/admin, and if this is so, set "allow=yes" cookie and
	 * redirect back to the page where we have been redirected to login.
	 */
	if (open_url != NULL ) {
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "open_url is %s<br>"
            ,open_url);

            if(__fp_openurl)__fp_openurl(__ref,open_url);
	} else {
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "Please login (enter admin/admin to pass)<br>"
		    "<form method=post>"
		    "Name: <input type=text name=name></input><br/>"
		    "Password: <input type=password name=pass></input><br/>"
		    "<input type=submit value=Login></input>"
		    "</form>");
	}

	if (open_url != NULL)
		mg_free(open_url);
}

static void
share_pkg(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
     send_file_by_path(conn,g_pkgPath);
}

static void
shs_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{

		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "Welcome to redwinner's mobileshare<br>"
		    "<a href=\"share.apk\">Download MobileShare apk</a><br><br>"
		    "<a href=\"upload.htm\">Upload File</a><br><br>"
		    "<a href=\"mailto:shaohongsheng@gmail.com\">email:shaohongsheng (at) gmail.com</a><br>"
		    "<a href=\"http://weibo.com/redwinner\">Redwinner's Weibo </a><br>"
		    "<a href=\"r.html\">Calendar</a><br><br>"
		    "<a href=\"bbs.html\">BBS</a><br>"
		    );
}

static struct mg_context	*ctx = NULL;

static void
upload_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
	save_post_file_data_to_dir(conn, mg_get_option(ctx, "root"));

	return ;
}

static void
upload_input_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "<!DOCTYPE HTML>"
		    "<html>"
		    "<head>"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
			"<title>Upload file to MobileShare!</title>"
		    "</head>"
		    "<body>"
			"<form action=\"uploadfile\" method=\"post\" enctype =\"multipart/form-data\" runat=\"server\">" 
		    "<input id=\"file1\" runat=\"server\" name=\"UpLoadFile\" type=\"file\" /><br>" 
		    "<input type=\"submit\" name=\"Button\" value=\"upload\" id=\"Button\" />"
		    "</form>"
		    "</body>" 
		    "</html>");
}

#include "rili.c"

static void
rili_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html\r\n"
		    "Content-Length: %d\r\n"
		    "Connection: close\r\n\r\n"
		    ,sizeof(index_html));
		    
		mg_write(conn,index_html,sizeof(index_html) );   
}

#include "m.c"

static void
r_manifest_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html\r\n"
		    "Content-Length: %d\r\n"
		    "Connection: close\r\n\r\n"
		    ,sizeof(r_manifest));
		    
		mg_write(conn,r_manifest,sizeof(r_manifest) );   
}
static void
http302Jobs_mp4_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\n"
		    "Server: mobileshare\r\n"
		    "Location: http://z.com/t/MOVIE/Jobs.mp4\r\n"
		    "Cache-Control: max-age=7200\r\n"
		    "Connection: close\r\n"
				"Content-Length: 0\r\n\r\n");
}

static void
bbs_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
	append_msg_data_to_bbs_file(conn, mg_get_option(ctx, "root"));

	return ;
}

static void
bbs_input_page(struct mg_connection *conn,
		const struct mg_request_info *ri, void *data)
{
		/* Print login page */
		mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		    "content-Type: text/html\r\n\r\n"
		    "<!DOCTYPE HTML>\r\n"
		    "<html>\r\n"
		    "<head>\r\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
			"<title>POST Message to MobileShare!</title>\r\n"
		    "</head>\r\n"
		    "<body>\r\n"
			"<form action=\"bbs\" method=\"post\" enctype =\"multipart/form-data\" runat=\"server\">\r\n" 
		    "<textarea name=\"msg\" rows=\"2\" cols=\"20\" id=\"msg\" style=\"background-color:White;border-color:#C0C0FF;border-width:1px;border-style:Solid;height:93px;width:445px;\"></textarea><br>\r\n" 
		    "<input type=\"submit\" name=\"Button\" value=\"Send\" id=\"Button\" />\r\n"
		    "</form>\r\n"
		    "</body>\r\n" 
		    "</html>");
}

int
main_ztv_webui_init(int argc, char *argv[])
{
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'A') {
		if (argc != 6)
			show_usage_and_exit();
		exit(mg_edit_passwords(argv[2], argv[3], argv[4],argv[5]));
	}

	if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
		show_usage_and_exit();

#ifndef _WIN32
	(void) signal(SIGCHLD, signal_handler);
#endif /* _WIN32 */
	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);

	if ((ctx = mg_start()) == NULL) {
		(void) printf("%s\n", "Cannot initialize Mongoose context");
		exit(EXIT_FAILURE);
	}

	process_command_line_arguments(ctx, argv);
	if (mg_get_option(ctx, "ports") == NULL &&
	    mg_set_option(ctx, "ports", "8080") != 1)
		exit(EXIT_FAILURE);

	printf("Mongoose %s started on port(s) [%s], serving directory [%s]\n",
	    mg_version(),
	    mg_get_option(ctx, "ports"),
	    mg_get_option(ctx, "root"));

    mg_set_uri_callback(ctx, "/login", &login_page, NULL);
    mg_set_uri_callback(ctx, "/ztv", &ztv_page, NULL);
    mg_set_uri_callback(ctx, "/share.apk", &share_pkg,NULL);
    mg_set_uri_callback(ctx, "/shs", &shs_page, NULL);
    mg_set_uri_callback(ctx, "/about", &shs_page, NULL);
    mg_set_uri_callback(ctx, "/uploadfile", &upload_page, NULL);
    mg_set_uri_callback(ctx, "/upload.html", &upload_input_page, NULL);
    mg_set_uri_callback(ctx, "/upload.htm", &upload_input_page, NULL);    
    mg_set_uri_callback(ctx, "/r.html", &rili_page, NULL);
    mg_set_uri_callback(ctx, "/r.manifest", &r_manifest_page, NULL);        
    mg_set_uri_callback(ctx, "/http302Jobs.mp4", &http302Jobs_mp4_page, NULL);  
    mg_set_uri_callback(ctx, "/bbs.html", &bbs_input_page, NULL);  
    mg_set_uri_callback(ctx, "/bbs", &bbs_page, NULL);

	fflush(stdout);
/*	while (exit_flag == 0)
		sleep(1);

	(void) printf("Exiting on signal %d, "
	    "waiting for all threads to finish...", exit_flag);
	fflush(stdout);
	mg_stop(ctx);
	(void) printf("%s", " done.\n");   */

	return (EXIT_SUCCESS);
}
int
main_ztv_webui_uninit(void)
{
	(void) printf("Exiting on signal %d, "
	    "waiting for all threads to finish...", exit_flag);
	fflush(stdout);
	if(ctx)mg_stop(ctx);
	ctx = NULL;
	(void) printf("%s", " done.\n");

	return (EXIT_SUCCESS);
}
int ztv_webui_set_openurl_func(void *ref,void *fp)
{
  __fp_openurl = (FP_OPENURL_CB)fp;
  __ref = ref;
  return 0;
}

int ztv_webui_set_share_pkg_path(const char *pkgpath)
{
    strcpy(g_pkgPath,pkgpath);
    return 0;
}