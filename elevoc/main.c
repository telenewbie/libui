#include "../ui.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>

//#define ELEVOC_NS_PATH (strcat(getenv("HOME"),"/.elevocrc"))
#define ELEVOC_PROJ_ID 2020
#define ELEVOC_FILE_NAME "/.elevocrc"
#define SHM_SIZE 64
#define CONTENT_LEN 4

char *s_fullpath = NULL;
char * shmptr = NULL;
int s_maskValue = 0;
int s_enableValue = 0;
FILE* file = NULL;

int needSave(){
    if(!shmptr)return 0;
    if((int)shmptr[0] == s_maskValue && (int)shmptr[1] == s_enableValue)  return 0;
    return 1;
}
void writeFile(char * data,int len){
    if(needSave()){
        fseek(file,0,SEEK_SET);
        fwrite(data,1,len,file);
        printf("write file over\n");
    }
}

void writeShm(char* data,int len){
    if(!shmptr)return;
    memset(shmptr,0,len);
    strncpy(shmptr,data,len);
    printf("write shm over\n");
}

void onSetting(uiButton*b,void*data){
        if(!file){
            file = fopen(s_fullpath,"wb+");
            if(!file) perror("fopen error");
            printf("create file:%s\n",s_fullpath);
#if 0
            else fclose(file);
            file = fopen(ELEVOC_NS_PATH,"wb+");
#endif
        }
    if(!shmptr){
        key_t key = ftok(/*ELEVOC_NS_PATH*/getenv("HOME"),ELEVOC_PROJ_ID);
        printf("key 0x%08x\n",key);
        if(key == -1){
            perror("ftok");
            exit(-1);
        }
        int shmid = shmget(key,SHM_SIZE,IPC_CREAT|0666);
        if(shmid<0)
        {
            perror("shmget error");
            exit(-1);
        }
        printf("client shmget shmid: %d\n",shmid);
        shmptr = shmat(shmid,0,0);
        if(shmptr == (void *) -1){
            perror("shmat error");
            exit(-1);
        }
        memset(shmptr,0,SHM_SIZE);
    }
    // write date
    char content[CONTENT_LEN] = {0};
    memset(content,0,CONTENT_LEN);
    snprintf(content,2,"%c",(char)s_maskValue);
    snprintf(content+1,2,"%c",(char)s_enableValue);
    writeFile(content,CONTENT_LEN);
    writeShm(content,CONTENT_LEN);
}

void onSlider(uiSlider*s,void* data){
    s_maskValue = uiSliderValue(s);
}

void onCheckEnable(uiCheckbox*c,void* data){
    s_enableValue = uiCheckboxChecked(c);
}

int onClosing(uiWindow *w,void* data){
    if(file) {fclose(file); file = NULL;}
    if(shmptr) {shmdt(shmptr); shmptr = NULL;}
    if(s_fullpath) {free(s_fullpath); s_fullpath = NULL;}
	uiQuit();
	return 1;
}

void readFromfile(){
    FILE* fp = fopen(s_fullpath,"rb");
    
    if(!fp)
    {
        return;
    }
    char data [CONTENT_LEN] ={0};
    memset(data,0,CONTENT_LEN);
    int ret = fread(data,1,CONTENT_LEN,fp);
    printf("xxxxxxxxxxxxxxxxxxxxxx:%d\n",ret);
    
    if(ret != CONTENT_LEN) goto err;
    s_maskValue = (int)data[0];
    s_enableValue = (int)data[1];
    printf("readfrom file mask:%d,enable:%d\n",s_maskValue,s_enableValue);
err:
    fclose(fp);
}

void init(){
    int len = strlen(getenv("HOME")) + strlen(ELEVOC_FILE_NAME) + 1;
    s_fullpath = malloc(len); // to account for NULL terminator
    printf("create file path:%p\n",s_fullpath);
    memset(s_fullpath,0,len);
    strcpy(s_fullpath, getenv("HOME"));
    strcat(s_fullpath, ELEVOC_FILE_NAME);
}

int main(){
    init();
    printf("hello\n");
    readFromfile();

	uiInitOptions o;
    uiWindow *w;
    uiBox *vbox;
    uiButton *btn;
	uiCheckbox *cbEnable;
	uiSlider* smask;
	uiGroup *group;
	uiForm *settingForm;

    memset(&o, 0, sizeof (uiInitOptions));
    if (uiInit(&o) != NULL)
    {
        abort();
    }
	w = uiNewWindow("eleovc", 320, 240, 0);
	uiWindowSetMargined(w, 1);
	
	vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);
    uiWindowSetChild(w, uiControl(vbox));

	cbEnable = uiNewCheckbox("");
    uiCheckboxSetChecked(cbEnable,s_enableValue);
    uiCheckboxOnToggled(cbEnable,onCheckEnable,NULL);

	smask = uiNewSlider(0,100);
    uiSliderSetValue(smask,s_maskValue);
    uiSliderOnChanged(smask,onSlider,NULL);

	group = uiNewGroup("setting");
	uiGroupSetMargined(group, 1);
    uiBoxAppend(vbox, uiControl(group), 0);

	settingForm = uiNewForm();
    uiFormSetPadded(settingForm, 1);
	uiGroupSetChild(group, uiControl(settingForm));

	uiFormAppend(settingForm,
                "noise suppression",
	            uiControl(cbEnable),
                0);
	uiFormAppend(settingForm,
                "mask:",
	  	        uiControl(smask),
                0);

	btn = uiNewButton("ok");
	uiButtonOnClicked(btn,onSetting,NULL);
	uiBoxAppend(vbox,uiControl(btn),0);

	uiWindowOnClosing(w,onClosing,NULL);
	uiControlShow(uiControl(w));
	uiMain();
	return 0;
}
