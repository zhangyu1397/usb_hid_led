#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hiddev.h>

int open_dev(){
    int fd = 0;
    fd = open("/dev/hiddev0",O_RDWR);
    if(fd < 0){
        printf("open error!!!!!\n");
    }
    return fd;
}

int digitalpen_getconfiginfo(int fd)  
{  
  char name[1024];  
  int  index;  
  int  report_type,report_id; 
  unsigned int   field_index,usage_index; 
   
  struct hiddev_devinfo dinfo;  
  struct hiddev_collection_info cinfo;  
  struct hiddev_report_info rinfo;  
  struct hiddev_field_info finfo;  
  struct hiddev_usage_ref uref;  
   
  const char *collection_type_name[] = {"Physical", "Application", "Logical"};  
  const char *report_type_name[] = {"Input", "Output", "Feature"};  
   
  // get device name  
  memset(name, 0, sizeof(name));  
  if( ioctl(fd, HIDIOCGNAME(sizeof(name)), name) < 0) {;  
    return -1;  
  }  
   
  // get device info  
  memset(&dinfo, 0, sizeof(dinfo));  
  if( ioctl(fd, HIDIOCGDEVINFO, &dinfo) < 0) {  
    return -1;  
  }  
   
  printf("\n[Device Information]\n");  
  printf("device name      = %s\n", name);  
  printf("vendor id        = 0x%04x\n", dinfo.vendor & 0xffff);  
  printf("product id       = 0x%04x\n", dinfo.product & 0xffff);  
  printf("version          = 0x%04x\n", dinfo.version & 0xffff);  
  printf("num_applications = 0x%x\n",   dinfo.num_applications);  
   
  // get collection info  
  printf("\n[Collections Information]\n");  
   
  index = 0;  
  while(1) {  
    memset(&cinfo, 0, sizeof(cinfo));  
    cinfo.index = index;  
   
    if(ioctl(fd, HIDIOCGCOLLECTIONINFO, &cinfo) < 0)  
      break;  
    index++;  
   
    printf("index = %d\n", cinfo.index);  
    if(cinfo.type >= 0 && cinfo.type <= 2)  
      printf("type  = %s\n", collection_type_name[cinfo.type]);  
    else  
      printf("type  = %d\n", cinfo.type);  
    printf("usage = 0x%x\n", cinfo.usage);  
    printf("level = %d\n\n", cinfo.level);  
  }  
   
  // get reports info  
  printf("[Report Information]\n");  
  for(report_type = HID_REPORT_TYPE_MIN; report_type <= HID_REPORT_TYPE_MAX; report_type++) {  
   
    for(report_id = HID_REPORT_ID_FIRST; ;) {  
      memset(&rinfo, 0, sizeof(rinfo));  
      rinfo.report_type = report_type;  
      rinfo.report_id = report_id;  
      if(ioctl(fd, HIDIOCGREPORTINFO, &rinfo) < 0)  
        break;  
   
      printf("report_type = %s\n", report_type_name[rinfo.report_type - 1]);  
      printf("report_id   = %d\n", rinfo.report_id);  
      printf("num_fields  = %d\n", rinfo.num_fields);  
   
      // get field info  
      
      for(field_index = 0; field_index < rinfo.num_fields; field_index++) {  
        memset(&finfo, 0, sizeof(finfo));  
        finfo.report_type = rinfo.report_type;  
        finfo.report_id   = rinfo.report_id;  
        finfo.field_index = field_index;  
   
        if(ioctl(fd, HIDIOCGFIELDINFO, &finfo) < 0)  
          break;  
        printf("\n    [field information]\n");  
        printf("    field_index   = %d\n",   field_index);  
        printf("    maxusage      = %d\n",   finfo.maxusage);  
        printf("    flags         = %d\n",   finfo.flags);  
        printf("    physical      = %d\n",   finfo.physical);  
        printf("    logical       = %d\n",   finfo.logical);  
        printf("    application   = 0x%x\n", finfo.application);  
        printf("    unit_exponent = %d\n",   finfo.unit_exponent);  
        printf("    unit          = %d\n",   finfo.unit);  
        printf("    logical_minimum  = %d; logical_maximum  = %d\n", finfo.logical_minimum, finfo.logical_maximum);  
        printf("    physical_minimum = %d; physical_maximum = %d\n", finfo.physical_minimum, finfo.physical_maximum);  
   
        // get usage info  
        printf("\n        [Usage code information]\n");  
        for(usage_index = 0; usage_index < finfo.maxusage; usage_index++) {  
          memset(&uref, 0, sizeof(uref));  
          uref.report_type = rinfo.report_type;  
          uref.report_id   = rinfo.report_id;  
          uref.field_index = field_index;  
          uref.usage_index = usage_index;  
          if(ioctl(fd, HIDIOCGUCODE, &uref) < 0)  
            break;  
   
          printf("        usage_index = %d, usage_code = 0x%x, value = %d\n",  uref.usage_index, uref.usage_code, uref.value);  
        }  
      }  
   
      report_id = rinfo.report_id | HID_REPORT_ID_NEXT;  
      printf("\n");  
   
    }  
  }  
  return 0;
}

#if 0
int hidSetOutReport(int fd, unsigned char * data, unsigned int len)
{
struct hiddev_usage_ref uref;
struct hiddev_report_info rinfo;
struct hiddev_field_info finfo;
unsigned int i,j;
int ret;

rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
rinfo.report_id = HID_REPORT_ID_FIRST;
ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);

if(ret<0)
{
printf("hid-ioctl-outrepinfo\n");
return -1;
}

// Get Fields
for (i = 0; i < rinfo.num_fields; i++)
{
finfo.report_type = rinfo.report_type;
finfo.report_id   = rinfo.report_id;
finfo.field_index = i;
ioctl(fd, HIDIOCGFIELDINFO, &finfo);

// Get usages
printf("(OUT Max Usage: %d/report id:%d) ", finfo.maxusage,finfo.report_id);
// si len es menor a finfo.maxusage, mando len bytes y completo con ceros
if(len < finfo.maxusage) finfo.maxusage = len;

for (j = 0; j < finfo.maxusage; j++)
{
uref.report_type = finfo.report_type;
uref.report_id   = finfo.report_id;
uref.field_index = i;
uref.usage_index = j;
uref.value = data[j];
ioctl(fd,HIDIOCSUSAGE, &uref);
}

//send report
ret = ioctl(fd,HIDIOCSREPORT,&rinfo);
if(ret<0)
{
printf("ioctl-setout\n");
return -1;
}
}

return finfo.maxusage;
}

#endif

int hidSetMicMute(int fd, int id, int val){
	int ret = 0;
	struct hiddev_usage_ref uref;
	struct hiddev_report_info rinfo;
	memset(&uref, 0 ,sizeof(struct hiddev_usage_ref));
	
	uref.report_type = HID_REPORT_TYPE_OUTPUT;
	uref.report_id   = id;
	uref.field_index = 0;
	uref.usage_index = 0;
	uref.value = val;
	
	ret = ioctl(fd,HIDIOCSUSAGE, &uref);
	if(ret<0)
	{
		printf("ioctl-setout HIDIOCSUSAGE\n");
		return -1;
	}
	
	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id = id;
	ret = ioctl(fd,HIDIOCSREPORT,&rinfo);
	if(ret < 0)
	{
		printf("ioctl-setout HIDIOCSREPORT\n");
		return -1;
	}
	return 0;
}


int main(int argc, char * argv[]){
    int fd,id = 9,val = 1;
    //unsigned char val[7] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01};
    fd = open_dev();
    if(argc == 3){
        id = strtol(argv[1],NULL,10);
        val = strtol(argv[2], NULL,10);
        printf("---------id:%d---val:%d-------\n",id,val);
    }else{
        if(fd > 0)
            digitalpen_getconfiginfo(fd);
    }

    if(fd > 0){
	hidSetMicMute(fd,id, val);
        close(fd);
    }
    return 0;
}


