
#ifndef MG_STUB_H
#define	MG_STUB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
typedef int (*FP_OPENURL_CB)(void *ref,const char *url);

int main_ztv_webui_init(int argc, char *argv[]);
int main_ztv_webui_uninit(void);
int ztv_webui_set_openurl_func(void *ref,void *fp);
int ztv_webui_set_share_pkg_path(const char *pkgpath);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
